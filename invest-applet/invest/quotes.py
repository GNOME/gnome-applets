import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gobject
from gettext import gettext as _
import gtk, gtk.glade, gobject, gnomevfs
import csv, os

import invest, invest.about, invest.chart

class QuoteUpdater(gtk.ListStore):
	__gsignals__ = {
		"quotes-updated" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, []),
	}
	
	SYMBOL, TICKER_ONLY, BALANCE, BALANCE_PCT, VALUE, VARIATION = range(6)
	def __init__ (self):
		gtk.ListStore.__init__ (self, gobject.TYPE_STRING, bool, float, float, float, float)
		gobject.timeout_add(invest.AUTOREFRESH_TIMEOUT, self.refresh)
		
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
					result[fields[0]][field[0]] = field[1](fields[i])
				else:
					result[fields[0]][field] = fields[i]
					
		return result 

	def populate(self, quotes):
		self.clear()
		
		#current = sum([sum([purchase["amount"]*val["trade"] for purchase in invest.STOCKS[ticker]]) for ticker, val in quotes.items()])
		#paid = sum([sum([purchase["amount"]*purchase["bought"]+purchase["comission"] for purchase in invest.STOCKS[ticker]]) for ticker in quotes.keys()])
		#balance = current - paid	
				
		#self.append([_("Total"), balance, balance/paid*100, current, 0])
		
		for ticker, val in quotes.items():
			# Check whether the symbol is a syimple quote, or a portfolio value
			is_simple_quote = True
			for purchase in invest.STOCKS[ticker]:
				if purchase["amount"] != 0:
					is_simple_quote = False
					break

			if is_simple_quote:
				self.append([ticker, True, 0, 0, val["trade"], val["variation"]])
			else:
				current = sum([purchase["amount"]*val["trade"] for purchase in invest.STOCKS[ticker] if purchase["amount"] != 0])
				paid = sum([purchase["amount"]*purchase["bought"] + purchase["comission"] for purchase in invest.STOCKS[ticker] if purchase["amount"] != 0])
				balance = current - paid
				
				self.append([ticker, False, balance, balance/paid*100, val["trade"], val["variation"]])
				
		self.emit("quotes-updated")

if gtk.pygtk_version < (2,8,0):
	gobject.type_register(QuoteUpdater)

_updater = QuoteUpdater()
def get_quotes_updater():
	return _updater
