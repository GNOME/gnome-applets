from os.path import join, getmtime
from gi.repository import GObject, Gtk, Gdk, GdkPixbuf, GConf, PanelApplet
from gettext import gettext as _
import csv
import locale
from urllib import urlopen
import datetime
from threading import Thread
from os import listdir, unlink
import re
import invest, invest.about, invest.chart
import currencies

# TODO: start currency retrieval after _all_ index expansion completed !!!

CHUNK_SIZE = 512*1024 # 512 kB
AUTOREFRESH_TIMEOUT = 15*60*1000 # 15 minutes

QUOTES_URL="http://finance.yahoo.com/d/quotes.csv?s=%(s)s&f=snc4l1d1t1c1ohgv&e=.csv"

# Sample (09/2/2010): "UCG.MI","UNICREDIT","EUR","UCG.MI",1.9410,"2/9/2010","6:10am",+0.0210,1.9080,1.9810,1.8920,166691232
QUOTES_CSV_FIELDS=["ticker", "label", "currency", ("trade", float), "date", "time", ("variation", float), ("open", float), ("high", float), ("low", float), ("volume", int)]

# based on http://www.johnstowers.co.nz/blog/index.php/2007/03/12/threading-and-pygtk/
class _IdleObject(GObject.Object):
	"""
	Override gobject.Object to always emit signals in the main thread
	by emmitting on an idle handler
	"""
	def __init__(self):
		GObject.Object.__init__(self)

	def emit(self, *args):
		GObject.idle_add(GObject.Object.emit,self,*args)

class QuotesRetriever(Thread, _IdleObject):
	"""
	Thread which uses gobject signals to return information
	to the GUI.
	"""
	__gsignals__ =  {
			"completed": (
				GObject.SignalFlags.RUN_LAST, GObject.TYPE_NONE, []),
			# FIXME: We don't monitor progress, yet ...
			#"progress": (
			#	gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, [
			#	gobject.TYPE_FLOAT])        #percent complete
			}

	def __init__(self, tickers):
		Thread.__init__(self)
		_IdleObject.__init__(self)
		self.tickers = tickers
		self.retrieved = False
		self.data = []
		self.currencies = []

	def run(self):
		quotes_url = QUOTES_URL % {"s": self.tickers}
		invest.debug("QuotesRetriever started: %s" % quotes_url);
		try:
			quotes_file = urlopen(quotes_url, proxies = invest.PROXY)
			self.data = quotes_file.read ()
			quotes_file.close ()
		except Exception, msg:
			invest.debug("Error while retrieving quotes data (url = %s): %s" % (quotes_url, msg))
		else:
			self.retrieved = True
		self.emit("completed")


class QuoteUpdater(Gtk.TreeStore):
	updated = False
	last_updated = None
	quotes_valid = False
	timeout_id = None
	SYMBOL, LABEL, CURRENCY, TICKER_ONLY, BALANCE, BALANCE_PCT, VALUE, VARIATION_PCT, PB = range(9)
	def __init__ (self, change_icon_callback, set_tooltip_callback):
		Gtk.TreeStore.__init__ (self, GObject.TYPE_STRING, GObject.TYPE_STRING, GObject.TYPE_STRING, bool, float, float, float, float, GdkPixbuf.Pixbuf)
		self.set_update_interval(AUTOREFRESH_TIMEOUT)
		self.change_icon_callback = change_icon_callback
		self.set_tooltip_callback = set_tooltip_callback
		self.set_sort_column_id(1, Gtk.SortType.ASCENDING)
		self.load_quotes()		# read the last cached quotes file
		self.load_all_index_quotes()	# expand indices if requested
		self.load_currencies()		# convert currencies from cached file
		self.refresh()			# download a new quotes file, this may fail if disconnected

		# tell the network manager to notify me when network status changes
		invest.nm.set_statechange_callback(self.nm_state_changed)

	# loads the cached csv file and its last-modification-time as self.last_updated
	def load_quotes(self):
		invest.debug("Loading quotes");
		try:
			f = open(invest.QUOTES_FILE, 'r')
			data = f.readlines()
			f.close()

			self.populate(self.parse_yahoo_csv(csv.reader(data)))
			self.updated = True
			self.last_updated = datetime.datetime.fromtimestamp(getmtime(invest.QUOTES_FILE))
			self.update_tooltip()
		except Exception, msg:
			invest.error("Could not load the cached quotes file %s: %s" % (invest.QUOTES_FILE, msg) )

	# stores the csv content on disk so it can be used on next start up
	def save_quotes(self, data):
		invest.debug("Storing quotes")
		try:
			f = open(invest.QUOTES_FILE, 'w')
			f.write(data)
			f.close()
		except Exception, msg:
			invest.error("Could not save the retrieved quotes file to %s: %s" % (invest.QUOTES_FILE, msg) )



	def expand_indices(self):
		if not ( invest.CONFIG.has_key('indexexpansion') and invest.CONFIG['indexexpansion'] ):
			# retrieve currencies immediately
			self.retrieve_currencies()
			return

		# trigger retrieval for each index
		for index in self.get_indices(invest.STOCKS):
			quotes_retriever = QuotesRetriever("@%s" % index)
			quotes_retriever.connect("completed", self.on_index_quote_retriever_completed, index)
			quotes_retriever.start()

	def on_index_quote_retriever_completed(self, retriever, index):
		if retriever.retrieved == False:
			invest.error("Failed to retrieve quotes for index %s!" % index)
		else:
			self.save_index_quotes(index, retriever.data)
			self.load_index_quotes(index)
			self.retrieve_currencies()

	def save_index_quotes(self, index, data):
		# store the index quotes
		invest.debug("Storing quotes of index %s" % index)
		try:
			filename = invest.INDEX_QUOTES_FILE_TEMPLATE.replace('#', index)
			filename = join(invest.USER_INVEST_DIR, filename)
			f = open(filename, 'w')
			f.write(data)
			f.close()
		except Exception, msg:
			invest.error("Could not save the retrieved index quotes file of %s to %s: %s" % (index, filename, msg) )
			return

	def load_index_quotes(self, index):
		# load the file
		try:
			filename = invest.INDEX_QUOTES_FILE_TEMPLATE.replace('#', index)
			filename = join(invest.USER_INVEST_DIR, filename)
			f = open(filename, 'r')
			data = f.readlines()
			f.close()
		except Exception, msg:
			invest.error("Could not load index quotes file %s of index %s: %s" % (filename, index, msg) )
			return

		# expand the index
		self.expand_index(index, data)

	def load_all_index_quotes(self):
		if not ( invest.CONFIG.has_key('indexexpansion') and invest.CONFIG['indexexpansion'] ):
			return

		# load all existing index quotes files
		files = listdir(invest.USER_INVEST_DIR)
		for file in files:
			# is this a index quote file?
			m = re.match(invest.INDEX_QUOTES_FILE_TEMPLATE.replace('#', '([^.]+)'), file)
			if m:
				index = m.group(1)
				filename = join(invest.USER_INVEST_DIR, file)

				# load the file
				f = open(filename, 'r')
				data = f.readlines()
				f.close()

				# expand respective indices
				if not self.expand_index(index, data):
					# delete index file because the index is not used anymore
					unlink(filename)

	def expand_index(self, index, data):
		invest.debug("Expanding index %s" % index)

		quotes = self.parse_yahoo_csv(csv.reader(data))

		nodes = self.find_stock(index)
		for node in nodes:
			for ticker in quotes.keys():
				quote = self.get_quote(quotes, ticker)
				row = self.insert(self.get_iter(node), 0, [ticker, quote["label"], quote["currency"], True, 0.0, 0.0, float(quote["trade"]), float(quote["variation_pct"]), None])
				self.retrieve_image(ticker, row)

		# indicate if index was found
		return len(nodes) > 0


	def set_update_interval(self, interval):
		if self.timeout_id != None:
			invest.debug("Canceling refresh timer")
			GObject.source_remove(self.timeout_id)
			self.timeout_id = None
		if interval > 0:
			invest.debug("Setting refresh timer to %s:%02d.%03d" % ( interval / 60000, interval % 60000 / 1000, interval % 1000) )
			self.timeout_id = GObject.timeout_add(interval, self.refresh)

	def nm_state_changed(self):
		# when nm is online but we do not have an update timer, create it and refresh
		if invest.nm.online():
			if self.timeout_id == None:
				self.set_update_interval(AUTOREFRESH_TIMEOUT)
				self.refresh()

	def refresh(self):
		invest.debug("Refreshing")

		# when nm tells me I am offline, stop the update interval
		if invest.nm.offline():
			invest.debug("We are offline, stopping update timer")
			self.set_update_interval(0)
			return False

		if len(invest.STOCKS) == 0:
			invest.debug("No stocks configured")
			return True

		tickers = '+'.join( self.get_tickers(invest.STOCKS) )
		quotes_retriever = QuotesRetriever(tickers)
		quotes_retriever.connect("completed", self.on_retriever_completed)
		quotes_retriever.start()

		return True

	def get_tickers(self, stocks):
		tickers = []
		for stock in stocks:
			if stock.has_key('ticker'):
				ticker = stock['ticker']
				tickers.append(ticker)
			else:
				tickers.extend(self.get_tickers(stock['list']))
		return tickers

	def get_indices(self, stocks):
		indices = []
		for stock in stocks:
			if stock.has_key('ticker'):
				ticker = stock['ticker']
				if ticker.startswith('^'):
					indices.append(ticker)
			else:
				indices.extend(self.get_indices(stock['list']))
		return indices

	def find_stock(self, symbol):
		list = []
		self.foreach(self.find_stock_cb, (symbol, list))
		return list

	def find_stock_cb(self, model, path, iter, data):
		(symbol, list) = data
		if model[path][0] == symbol:
			list.append(str(path))

	# locale-aware formatting of the percent float (decimal point, thousand grouping point) with 2 decimal digits
	def format_percent(self, value):
		return locale.format("%+.2f", value, True) + "%"

	# locale-aware formatting of the float value (decimal point, thousand grouping point) with sign and 2 decimal digits
	def format_difference(self, value):
		return locale.format("%+.2f", value, True, True)

	def on_retriever_completed(self, retriever):
		if retriever.retrieved == False:
			invest.debug("QuotesRetriever failed for tickers '%s'" % retriever.tickers);
			self.update_tooltip(_('Invest could not connect to Yahoo! Finance'))
		else:
			invest.debug("QuotesRetriever completed");
			# cache the retrieved csv file
			self.save_quotes(retriever.data)
			# load the cache and parse it
			self.load_quotes()
			# expand index values if requested
			self.expand_indices()


	def on_currency_retriever_completed(self, retriever):
		if retriever.retrieved == False:
			invest.error("Failed to retrieve currency rates!")
		else:
			self.save_currencies(retriever.data)
			self.load_currencies()
		self.update_tooltip()

	def save_currencies(self, data):
		invest.debug("Storing currencies to %s" % invest.CURRENCIES_FILE)
		try:
			f = open(invest.CURRENCIES_FILE, 'w')
			f.write(data)
			f.close()
		except Exception, msg:
			invest.error("Could not save the retrieved currencies to %s: %s" % (invest.CURRENCIES_FILE, msg) )

	def load_currencies(self):
		invest.debug("Loading currencies from %s" % invest.CURRENCIES_FILE)
		try:
			f = open(invest.CURRENCIES_FILE, 'r')
			data = f.readlines()
			f.close()

			self.convert_currencies(self.parse_yahoo_csv(csv.reader(data)))
		except Exception, msg:
			invest.error("Could not load the currencies from %s: %s" % (invest.CURRENCIES_FILE, msg) )

	def update_tooltip(self, msg = None):
		tooltip = []
		if self.quotes_count > 0:
			# Translators: This is share-market jargon. It is the average percentage change of all stock prices. The %s gets replaced with the string value of the change (localized), including the percent sign.
			tooltip.append(_('Average change: %s') % self.format_percent(self.avg_quotes_change))
		for currency, stats in self.statistics.items():
			# get the statsitics
			balance = stats["balance"]
			paid = stats["paid"]
			change = self.format_percent(balance / paid * 100)
			balance = self.format_difference(balance)

			# Translators: This is share-market jargon. It refers to the total difference between the current price and purchase price for all the shares put together for a particular currency. i.e. How much money would be earned if they were sold right now. The first string is the change value, the second the currency, and the third value is the percentage of the change, formatted using user's locale.
			tooltip.append(_('Positions balance: %s %s (%s)') % (balance, currency, change))
		if self.last_updated != None:
			tooltip.append(_('Updated at %s') % self.last_updated.strftime("%H:%M"))
		if msg != None:
			tooltip.append(msg)
		self.set_tooltip_callback('\n'.join(tooltip))


	def parse_yahoo_csv(self, csvreader):
		result = {}
		for fields in csvreader:
			if len(fields) == 0:
				continue

			if len(fields) != len(QUOTES_CSV_FIELDS):
				invest.debug("CSV line has unexpected number of fields, expected %d, has %d: %s" % (len(QUOTES_CSV_FIELDS), len(fields), fields))
				continue

			result[fields[0]] = {}
			for i, field in enumerate(QUOTES_CSV_FIELDS):
				if type(field) == tuple:
					try:
						result[fields[0]][field[0]] = field[1](fields[i])
					except:
						result[fields[0]][field[0]] = 0
				else:
					result[fields[0]][field] = fields[i]
			# calculated fields
			try:
				result[fields[0]]['variation_pct'] = result[fields[0]]['variation'] / float(result[fields[0]]['trade'] - result[fields[0]]['variation']) * 100
			except ZeroDivisionError:
				result[fields[0]]['variation_pct'] = 0
		return result

	# Computes the balance of the given purchases using a certain current value
	# and optionally a current exchange rate.
	def balance(self, purchases, value, currentrate=None):
		current = 0
		paid = 0

		for purchase in purchases:
			if purchase["amount"] != 0:
				buyrate = purchase["exchange"]
				# if the buy rate is invalid, use 1.0
				if buyrate <= 0:
					buyrate = 1.0

				# if no current rate is given, ignore buy rate
				if currentrate == None:
					buyrate = 1.0
					rate = 1.0
				else:
					# otherwise, take use buy rate and current rate to compute the balance
					rate = currentrate

				# current value is the current rate * amount * value
				current += rate * purchase["amount"] * value
				# paid is buy rate * ( amount * price + commission )
				paid += buyrate * (purchase["amount"] * purchase["bought"] + purchase["comission"])

		balance = current - paid
		if paid != 0:
			change = 100*balance/paid
		else:
			change = 100 # Not technically correct, but it will look more intuitive than the real result of infinity.

		return (balance, change)

	def populate(self, quotes):
		if (len(quotes) == 0):
			return

		self.clear()
		self.currencies = []

		try:
			quote_items = quotes.items ()
			quote_items.sort ()

			self.quotes_change = 0
			self.quotes_count = 0
			self.statistics = {}

			# iterate over the STOCKS tree and build up this treestore
			self.add_quotes(quotes, invest.STOCKS, None)

			# we can only compute an avg quote change if there are quotes
			if self.quotes_count > 0:
				self.avg_quotes_change = self.quotes_change/float(self.quotes_count)

				# change icon
				quotes_change_sign = 0
				if self.avg_quotes_change != 0:
					quotes_change_sign = self.avg_quotes_change / abs(self.avg_quotes_change)
				self.change_icon_callback(quotes_change_sign)
			else:
				self.avg_quotes_change = 0

			# mark quotes to finally be valid
			self.quotes_valid = True

		except Exception, msg:
			invest.debug("Failed to populate quotes: %s" % msg)
			invest.debug(quotes)
			self.quotes_valid = False


	def retrieve_currencies(self):
		# start retrieving currency conversion rates
		if invest.CONFIG.has_key("currency"):
			target_currency = invest.CONFIG["currency"]
			symbols = []

			invest.debug("These currencies occur: %s" % self.currencies)
			for currency in self.currencies:
				if currency == target_currency:
					continue

				invest.debug("%s will be converted to %s" % ( currency, target_currency ))
				symbol = currency + target_currency + "=X"
				symbols.append(symbol)

			if len(symbols) > 0:
				tickers = '+'.join(symbols)
				quotes_retriever = QuotesRetriever(tickers)
				quotes_retriever.connect("completed", self.on_currency_retriever_completed)
				quotes_retriever.start()


	def add_quotes(self, quotes, stocks, parent):
		for stock in stocks:
			if not stock.has_key('ticker'):
				name = stock['name']
				list = stock['list']
				# here, the stock group name is used as the label,
				# so in quotes, the key == None indicates a group
				# in preferences, the label == None indicates this
				try:
					row = self.insert(parent, 0, [None, name, None, True, None, None, None, None, None])
				except Exception, msg:
					invest.debug("Failed to insert group %s: %s" % (name, msg))
				self.add_quotes(quotes, list, row)
				# Todo: update the summary statistics of row
			else:
				ticker = stock['ticker'];
				if not quotes.has_key(ticker):
					invest.debug("no quote for %s retrieved" % ticker)
					continue

				# get the quote
				quote = self.get_quote(quotes, ticker)

				# get the label of this stock for later reuse
				label = stock["label"]
				if len(label) == 0:
					if len(quote["label"]) != 0:
						label = quote["label"]
					else:
						label = ticker

				# Check whether the symbol is a simple quote, or a portfolio value
				try:
					if self.is_simple_quote(stock):
						row = self.insert(parent, 0, [ticker, label, quote["currency"], True, 0.0, 0.0, float(quote["trade"]), float(quote["variation_pct"]), None])
					else:
						(balance, change) = self.balance(stock["purchases"], quote["trade"])
						row = self.insert(parent, 0, [ticker, label, quote["currency"], False, float(balance), float(change), float(quote["trade"]), float(quote["variation_pct"]), None])
						self.add_balance_change(balance, change, quote["currency"])
				except Exception, msg:
					invest.debug("Failed to insert stock %s: %s" % (stock, msg))

				self.quotes_change += quote['variation_pct']
				self.quotes_count += 1

				self.retrieve_image(ticker, row)

	def retrieve_image(self, ticker, row):
		if invest.CONFIG.has_key('hidecharts') and invest.CONFIG['hidecharts']:
			return

		url = 'http://ichart.yahoo.com/h?s=%s' % ticker
		image_retriever = invest.chart.ImageRetriever(url)
		image_retriever.connect("completed", self.set_pb_callback, row)
		image_retriever.start()

	def get_quote(self, quotes, ticker):
		# the data for this quote
		quote = quotes[ticker];

		# make sure the currency field is upper case
		quote["currency"] = quote["currency"].upper();

		# the currency of currency conversion rates like EURUSD=X is wrong in csv
		# this can be fixed easily by reusing the latter currency in the symbol
		if len(ticker) == 8 and ticker.endswith("=X"):
			quote["currency"] = ticker[3:6]

		# indices should not have a currency, though yahoo says so
		if ticker.startswith("^"):
			quote["currency"] = ""

		# sometimes, funny currencies are returned (special characters), only consider known currencies
		if len(quote["currency"]) > 0 and quote["currency"] not in currencies.Currencies.currencies:
			invest.debug("Currency '%s' is not known, dropping" % quote["currency"])
			quote["currency"] = ""

		# if this is a currency not yet seen and different from the target currency, memorize it
		if quote["currency"] not in self.currencies and len(quote["currency"]) > 0:
			self.currencies.append(quote["currency"])

		return quote

	def convert_currencies(self, quotes):
		# if there is no target currency, this method should never have been called
		if not invest.CONFIG.has_key("currency"):
			return

		# reset the overall balance
		self.statistics = {}

		# collect the rates for the currencies
		rates = {}
		for symbol, data in quotes.items():
			currency = symbol[0:3]
			rate = data["trade"]
			rates[currency] = rate

		# convert all non target currencies
		target_currency = invest.CONFIG["currency"]
		iter = self.get_iter_first()
		while iter != None:
			currency = self.get_value(iter, self.CURRENCY)
			if currency == None:
				iter = self.iter_next(iter)
				continue
			symbol = self.get_value(iter, self.SYMBOL)
			# ignore stocks that are currency conversions
			# and only convert stocks that are not in the target currency
			# and if we have a conversion rate
			if not ( len(symbol) == 8 and symbol[6:8] == "=X" ) and \
			   currency != target_currency and rates.has_key(currency):
				# first convert the balance, it needs the original value
				if not self.get_value(iter, self.TICKER_ONLY):
					ticker = self.get_value(iter, self.SYMBOL)
					value = self.get_value(iter, self.VALUE)
					(balance, change) = self.balance(invest.STOCKS[ticker]["purchases"], value, rates[currency])
					self.set_value(iter, self.BALANCE, balance)
					self.set_value(iter, self.BALANCE_PCT, change)
					self.add_balance_change(balance, change, target_currency)

				# now, convert the value
				value = self.get_value(iter, self.VALUE)
				value *= rates[currency]
				self.set_value(iter, self.VALUE, value)
				self.set_value(iter, self.CURRENCY, target_currency)

			else:
				# consider non-converted stocks here
				balance = self.get_value(iter, self.BALANCE)
				change  = self.get_value(iter, self.BALANCE_PCT)
				self.add_balance_change(balance, change, currency)

			iter = self.iter_next(iter)

	def add_balance_change(self, balance, change, currency):
		if balance == 0 and change == 0:
			return

		if self.statistics.has_key(currency):
			self.statistics[currency]["balance"] += balance
			self.statistics[currency]["paid"] += balance/change*100
		else:
			self.statistics[currency] = { "balance" : balance, "paid" : balance/change*100 }

	def set_pb_callback(self, retriever, row):
		self.set_value(row, self.PB, retriever.image.get_pixbuf())

	# check if we have only simple quotes
	def simple_quotes_only(self, stocks):
		for stock in stocks:
			if stock.has_key('purchases'):
				if not self.is_simple_quote(stock):
					return False
			else:
				if not self.simple_quotes_only(stock['list']):
					return False
		return True

	def is_simple_quote(self, stock):
		for purchase in stock["purchases"]:
			if purchase["amount"] != 0:
				return False
		return True
