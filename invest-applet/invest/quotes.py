from os.path import join
import gnomeapplet, gtk, gtk.gdk, gconf, gobject
from gettext import gettext as _
import csv
from urllib import urlopen
import datetime
from threading import Thread

import invest, invest.about, invest.chart

CHUNK_SIZE = 512*1024 # 512 kB
AUTOREFRESH_TIMEOUT = 15*60*1000 # 15 minutes

QUOTES_URL="http://finance.yahoo.com/d/quotes.csv?s=%(s)s&f=sl1d1t1c1ohgv&e=.csv"

# Sample (25/4/2008): UCG.MI,"4,86",09:37:00,2008/04/25,"0,07","4,82","4,87","4,82",11192336
QUOTES_CSV_FIELDS=["ticker", ("trade", float), "time", "date", ("variation", float), ("open", float)]

# based on http://www.johnstowers.co.nz/blog/index.php/2007/03/12/threading-and-pygtk/
class _IdleObject(gobject.GObject):
	"""
	Override gobject.GObject to always emit signals in the main thread
	by emmitting on an idle handler
	"""
	def __init__(self):
		gobject.GObject.__init__(self)

	def emit(self, *args):
		gobject.idle_add(gobject.GObject.emit,self,*args)

class QuotesRetriever(Thread, _IdleObject):
	"""
	Thread which uses gobject signals to return information
	to the GUI.
	"""
	__gsignals__ =  {
			"completed": (
				gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, []),
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

	def run(self):
		quotes_url = QUOTES_URL % {"s": self.tickers}
		try:
			quotes_file = urlopen(quotes_url, proxies = invest.PROXY)
			self.data = quotes_file.readlines ()
			quotes_file.close ()
		except Exception, msg:
			invest.debug("Error while retrieving quotes data (url = %s): %s" % (quotes_url, msg))
		else:
			self.retrieved = True
		self.emit("completed")


class QuoteUpdater(gtk.ListStore):
	updated = False
	last_updated = None
	quotes_valid = False
	timeout_id = None
	SYMBOL, LABEL, TICKER_ONLY, BALANCE, BALANCE_PCT, VALUE, VARIATION_PCT, PB = range(8)
	def __init__ (self, change_icon_callback, set_tooltip_callback):
		gtk.ListStore.__init__ (self, gobject.TYPE_STRING, gobject.TYPE_STRING, bool, float, float, float, float, gtk.gdk.Pixbuf)
		self.set_update_interval(AUTOREFRESH_TIMEOUT)
		self.change_icon_callback = change_icon_callback
		self.set_tooltip_callback = set_tooltip_callback
		self.set_sort_column_id(1, gtk.SORT_ASCENDING)
		self.refresh()

		# tell the network manager to notify me when network status changes
		invest.nm.set_statechange_callback(self.nm_state_changed)

	def set_update_interval(self, interval):
		if self.timeout_id != None:
			invest.debug("Canceling refresh timer")
			gobject.source_remove(self.timeout_id)
			self.timeout_id = None
		if interval > 0:
			invest.debug("Setting refresh timer to %s:%02d.%03d" % ( interval / 60000, interval % 60000 / 1000, interval % 1000) )
			self.timeout_id = gobject.timeout_add(interval, self.refresh)

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
			return True

		tickers = '+'.join(invest.STOCKS.keys())
		quotes_retriever = QuotesRetriever(tickers)
		quotes_retriever.connect("completed", self.on_retriever_completed)
		quotes_retriever.start()

		return True


	def on_retriever_completed(self, retriever):
		if retriever.retrieved == False:
			tooltip = [_('Invest could not connect to Yahoo! Finance')]
			if self.last_updated != None:
				tooltip.append(_('Updated at %s') % self.last_updated.strftime("%H:%M"))
			self.set_tooltip_callback('\n'.join(tooltip))
		else:
			self.populate(self.parse_yahoo_csv(csv.reader(retriever.data)))
			self.updated = True
			self.last_updated = datetime.datetime.now()
			tooltip = []
			if self.simple_quotes_count > 0:
				# Translators: This is share-market jargon. It is the percentage change in the price of a stock. The %% gets changed to a single percent sign and the %+.2f gets replaced with the value of the change.
				tooltip.append(_('Quotes average change %%: %+.2f%%') % self.avg_simple_quotes_change)
			if self.positions_count > 0:
				# Translators: This is share-market jargon. It refers to the total difference between the current price and purchase price for all the shares put together. i.e. How much money would be earned if they were sold right now.
				tooltip.append(_('Positions balance: %+.2f') % self.positions_balance)
			tooltip.append(_('Updated at %s') % self.last_updated.strftime("%H:%M"))
			self.set_tooltip_callback('\n'.join(tooltip))



	def parse_yahoo_csv(self, csvreader):
		result = {}
		for fields in csvreader:
			if len(fields) == 0:
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

	def populate(self, quotes):
		if (len(quotes) == 0):
			return

		self.clear()
		
		try:
			quote_items = quotes.items ()
			quote_items.sort ()

			simple_quotes_change = 0
			self.simple_quotes_count = 0
			self.positions_balance = 0
			self.positions_count = 0

			for ticker, val in quote_items:
				pb = None

				# get the label of this stock for later reuse
				label = invest.STOCKS[ticker]["label"]
				if len(label) == 0:
					label = ticker

				# Check whether the symbol is a simple quote, or a portfolio value
				is_simple_quote = True
				for purchase in invest.STOCKS[ticker]["purchases"]:
					if purchase["amount"] != 0:
						is_simple_quote = False
						break

				if is_simple_quote:
					self.simple_quotes_count += 1
					row = self.insert(0, [ticker, label, True, 0, 0, val["trade"], val["variation_pct"], pb])
					simple_quotes_change += val['variation_pct']
				else:
					self.positions_count += 1
					current = sum([purchase["amount"]*val["trade"] for purchase in invest.STOCKS[ticker]["purchases"] if purchase["amount"] != 0])
					paid = sum([purchase["amount"]*purchase["bought"] + purchase["comission"] for purchase in invest.STOCKS[ticker]["purchases"] if purchase["amount"] != 0])
					balance = current - paid
					if paid != 0:
						change = 100*balance/paid
					else:
						change = 100 # Not technically correct, but it will look more intuitive than the real result of infinity.
					row = self.insert(0, [ticker, label, False, balance, change, val["trade"], val["variation_pct"], pb])
					self.positions_balance += balance

				if len(ticker.split('.')) == 2:
					url = 'http://ichart.europe.yahoo.com/h?s=%s' % ticker
				else:
					url = 'http://ichart.yahoo.com/h?s=%s' % ticker

				image_retriever = invest.chart.ImageRetriever(url)
				image_retriever.connect("completed", self.set_pb_callback, row)
				image_retriever.start()

			if self.simple_quotes_count > 0:
				self.avg_simple_quotes_change = simple_quotes_change/float(self.simple_quotes_count)
			else:
				self.avg_simple_quotes_change = 0

			if self.avg_simple_quotes_change != 0:
				simple_quotes_change_sign = self.avg_simple_quotes_change / abs(self.avg_simple_quotes_change)
			else:
				simple_quotes_change_sign = 0

			# change icon
			if self.simple_quotes_count > 0:
				self.change_icon_callback(simple_quotes_change_sign)
			else:
				positions_balance_sign = self.positions_balance/abs(self.positions_balance)
				self.change_icon_callback(positions_balance_sign)
				
			# mark quotes to finally be valid
			self.quotes_valid = True

		except Exception, msg:
			invest.debug("Failed to populate quotes: %s" % msg)
			invest.debug(quotes)
			self.quotes_valid = False

	def set_pb_callback(self, retriever, row):
		self.set_value(row, self.PB, retriever.image.get_pixbuf())

	# check if we have only simple quotes
	def simple_quotes_only(self):
		res = True
		for entry, data in invest.STOCKS.iteritems():
			purchases = data["purchases"]
			for purchase in purchases:
				if purchase["amount"] != 0:
					res = False
					break
		return res

if gtk.pygtk_version < (2,8,0):
	gobject.type_register(QuoteUpdater)
