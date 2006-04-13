import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gobject
from gettext import gettext as _
import gtk, gtk.glade, egg.trayicon, gobject, gnomevfs
import csv, os
from gettext import gettext as _
import invest, invest.about, invest.chart
from invest.quotes import get_quotes_updater
from invest import *

GREEN = "#73d216"
RED = "#ef2929"

class InvestWidget(gtk.TreeView):
	def __init__(self):
		gtk.TreeView.__init__(self)
				
		self.set_property("headers-visible", False)
		self.set_property("rules-hint", True)
		self.set_reorderable(True)
		
		cell = gtk.CellRendererText ()
		self.column_description = gtk.TreeViewColumn ("Description", cell)
		self.column_description.set_cell_data_func(cell, self._get_cell_data)
		
		self.append_column(self.column_description)
		self.connect('row-activated', self.on_row_activated)
		
		self.set_model(get_quotes_updater())
	
	def _get_cell_data(self, column, cell, model, iter):
		color = GREEN
		if model[iter][model.BALANCE] < 0:
			color = RED
						
		cell.set_property('markup',
			"<span face='Monospace'>%s: <span foreground='%s'>%+.2f$ (%+.2f%%)</span> %.2f$</span>" %
			(model[iter][model.SYMBOL], color, model[iter][model.BALANCE], model[iter][model.BALANCE_PCT], model[iter][model.VALUE]))
				
	def on_row_activated(self, treeview, path, view_column):
		ticker = self.get_model()[self.get_model().get_iter(path)][0]
		if ticker == None:
			return
		
		invest.chart.show_chart([ticker])

gobject.type_register(InvestWidget)
	
class InvestTicker(gtk.Label):
	def __init__(self):
		gtk.Label.__init__(self, _("Waiting..."))
		
		self.quotes = []
		gobject.timeout_add(TICKER_TIMEOUT, self.scroll_quotes)
		
		get_quotes_updater().connect('quotes-updated', self.on_quotes_update)
						
	def on_quotes_update(self, updater):
		self.quotes = []
		updater.foreach(self.update_quote, None)
	
	def update_quote(self, model, path, iter, user_data):
		color = GREEN
		if model[iter][model.BALANCE] < 0:
			color = RED
		
		self.quotes.append(
			"%s: <span foreground='%s'>%+.2f$ (%+.2f%%)</span> %.2f$" %
			(model[iter][model.SYMBOL], color, model[iter][model.BALANCE], model[iter][model.BALANCE_PCT], model[iter][model.VALUE]))
				
	def scroll_quotes(self):
		if len(self.quotes) == 0:
			return True
		
		q = self.quotes.pop()
		self.set_markup("<span face='Monospace'>%s</span>" % q)
		self.quotes.insert(0, q)
		
		return True

gobject.type_register(InvestTicker)
