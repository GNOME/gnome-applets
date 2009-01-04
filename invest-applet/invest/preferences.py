from gettext import gettext as _
from os.path import join
import gtk, gobject, gconf
import invest
from gettext import gettext as _

import cPickle

class PrefsDialog:
	def __init__(self, applet):
		self.ui = gtk.Builder()
		self.ui.add_from_file(join(invest.BUILDER_DATA_DIR, "prefs-dialog.ui"))

		self.dialog = self.ui.get_object("preferences")
		self.treeview = self.ui.get_object("stocks")
		
		self.ui.get_object("add").connect('clicked', self.on_add_stock)
		self.ui.get_object("add").connect('activate', self.on_add_stock)
		self.ui.get_object("remove").connect('clicked', self.on_remove_stock)
		self.ui.get_object("remove").connect('activate', self.on_remove_stock)
		self.treeview.connect('key-press-event', self.on_tree_keypress)

		self.typs = (str, float, float, float)
		self.names = (_("Symbol"), _("Amount"), _("Price"), _("Commission"))
		store = gtk.ListStore(*self.typs)
		self.treeview.set_model(store)
		self.model = store
		
		def on_cell_edited(cell, path, new_text, col, typ):
			try:
				store[path][col] = typ(new_text)
			except:
				pass
		
		def get_cell_data(column, cell, model, iter, data):
			typ, col = data
			if typ == int:
				cell.set_property('text', "%d" % typ(model[iter][col]))
			elif typ == float:
				cell.set_property('text', "%.2f" % typ(model[iter][col]))
			else:
				cell.set_property('text', typ(model[iter][col]))
			
		def create_cell (view, column, name, typ):
			cell_description = gtk.CellRendererText ()
			cell_description.set_property("editable", True)
			cell_description.connect("edited", on_cell_edited, column, typ)
			column_description = gtk.TreeViewColumn (name, cell_description)
			if typ == str:
				column_description.set_attributes (cell_description, text=0)
			if typ == float:
				column_description.set_cell_data_func(cell_description, get_cell_data, (float, column))
			view.append_column(column_description)
		

		for n in xrange (0, 4):
			create_cell (self.treeview, n, self.names[n], self.typs[n])		
		stock_items = invest.STOCKS.items ()
		stock_items.sort ()
		for key, purchases in stock_items:
			for purchase in purchases:
				store.append([key, purchase["amount"], purchase["bought"], purchase["comission"]])

		try:
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-16.png"), -1,-1)
			self.dialog.set_icon(pixbuf)
		except Exception, msg:
			pass
				
		self.sync_ui()
		
	def show_run_hide(self, explanation = ""):
		expl = self.ui.get_object("explanation")
		expl.set_markup(explanation)
		self.dialog.show_all()
		if explanation == "":
			expl.hide()
		self.dialog.run()
		self.dialog.destroy()
		
		invest.STOCKS = {}
		
		def save_symbol(model, path, iter):
			#if int(model[iter][1]) == 0 or float(model[iter][2]) < 0.0001:
			#	return
			
			if not model[iter][0] in invest.STOCKS:
				invest.STOCKS[model[iter][0]] = []
				
			invest.STOCKS[model[iter][0]].append({
				"amount": float(model[iter][1]),
				"bought": float(model[iter][2]),
				"comission": float(model[iter][3]),
			})
		self.model.foreach(save_symbol)
		try:
			cPickle.dump(invest.STOCKS, file(invest.STOCKS_FILE, 'w'))
			if invest.DEBUGGING: print 'Stocks written to file'
		except Exception, msg:
			if invest.DEBUGGING: print 'Could not save stocks file:', msg
			
	
	def sync_ui(self):
		pass
	
	def on_add_stock(self, w):
		self.treeview.get_model().append(["GOOG", 0, 0, 0])
		
	def on_remove_stock(self, w):
		model, paths = self.treeview.get_selection().get_selected_rows()
		for path in paths:
			model.remove(model.get_iter(path))
			
	def on_tree_keypress(self, w, event):
		if event.keyval == 65535:
			self.on_remove_stock(self, w)
			
		return False
		
def show_preferences(applet, explanation = ""):
	PrefsDialog(applet).show_run_hide(explanation)
