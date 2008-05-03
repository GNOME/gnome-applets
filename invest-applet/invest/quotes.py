import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gobject
from gettext import gettext as _
import gtk, gobject, gnomevfs
import csv, os

import invest, invest.about, invest.chart

class QuoteUpdater(gtk.ListStore):
	SYMBOL, TICKER_ONLY, BALANCE, BALANCE_PCT, VALUE, VARIATION_PCT = range(6)
	def __init__ (self, change_icon_callback):
		gtk.ListStore.__init__ (self, gobject.TYPE_STRING, bool, float, float, float, float, gtk.gdk.Pixbuf)
		gobject.timeout_add(invest.AUTOREFRESH_TIMEOUT, self.refresh)
		self.change_icon_callback = change_icon_callback
		self.refresh()
		
	def refresh(self):
		if len(invest.STOCKS) == 0:
			return True
			
		s = ""
		for ticker in invest.STOCKS.keys():
			s += "%s+" % ticker
		
		gnomevfs.async.open(invest.QUOTES_URL % {"s": s[:-1]}, self.on_quotes_open)
		return True
		
	def on_quotes_open(self, handle, exc_type):
		if not exc_type:
			handle.read(invest.GNOMEVFS_CHUNK_SIZE, lambda h,d,e,b: self.on_quotes_read(h,d,e,b, ""))
		else:
			handle.close(lambda *args: None)	

	def on_quotes_read(self, handle, data, exc_type, bytes_requested, read):
		if not exc_type:
			read += data
			
		if exc_type:
			handle.close(lambda *args: None)
			self.populate(self.parse_yahoo_csv(csv.reader(read.split("\n"))))
		else:
			handle.read(invest.GNOMEVFS_CHUNK_SIZE, lambda h,d,e,b: self.on_quotes_read(h,d,e,b, read))

	def parse_yahoo_csv(self, csvreader):
		result = {}
		for fields in csvreader:
			if len(fields) == 0:
				continue

			result[fields[0]] = {}
			for i, field in enumerate(invest.QUOTES_CSV_FIELDS):
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
		self.clear()
		
		quote_items = quotes.items ()
		quote_items.sort ()

		simple_quotes_change = 0
		simple_quotes_count = 0
		positions_balance = 0
		positions_count = 0

		for ticker, val in quote_items:
			pb = None
			
			# Check whether the symbol is a simple quote, or a portfolio value
			is_simple_quote = True
			for purchase in invest.STOCKS[ticker]:
				if purchase["amount"] != 0:
					is_simple_quote = False
					break
			
			if is_simple_quote:
				simple_quotes_count += 1
				row = self.insert(0, [ticker, True, 0, 0, val["trade"], val["variation_pct"], pb])
				simple_quotes_change += val['variation_pct']
			else:
				positions_count += 1
				current = sum([purchase["amount"]*val["trade"] for purchase in invest.STOCKS[ticker] if purchase["amount"] != 0])
				paid = sum([purchase["amount"]*purchase["bought"] + purchase["comission"] for purchase in invest.STOCKS[ticker] if purchase["amount"] != 0])
				balance = current - paid
				if paid != 0:
					change = 100*balance/paid
				else:
					change = 100 # Not technically correct, but it will look more intuitive than the real result of infinity.
				row = self.insert(0, [ticker, False, balance, change, val["trade"], val["variation_pct"], pb])
				positions_balance += balance
				
			invest.chart.FinancialSparklineChartPixbuf(ticker, self.set_pb_callback, row)

		if simple_quotes_count > 0:
			change = simple_quotes_change/float(simple_quotes_count)
			if change != 0:
				self.change_icon_callback(change/abs(change))
			else:
				self.change_icon_callback(change)
		else:
			self.change_icon_callback(positions_balance/abs(positions_balance))

	def set_pb_callback(self, pb, row):
		if pb != None:
			self.set_value(row, 6, pb)
	
	# check if we have only simple quotes
	def simple_quotes_only(self):
		res = True
		for entry, value in invest.STOCKS.iteritems():
			for purchase in value:
				if purchase["amount"] != 0:
					res = False
					break
		return res

if gtk.pygtk_version < (2,8,0):
	gobject.type_register(QuoteUpdater)

