from gettext import gettext as _
from os.path import join
import gtk, gtk.glade, gobject, gconf
import invest
from gettext import gettext as _

import cPickle

class PrefsDialog:
	def __init__(self, applet):
		self.glade = gtk.glade.XML(join(invest.SHARED_DATA_DIR, "prefs-dialog.glade"))

		self.dialog = self.glade.get_widget("preferences")
		self.treeview = self.glade.get_widget("stocks")
		
		self.glade.get_widget("add").connect('clicked', self.on_add_stock)
		self.glade.get_widget("add").connect('activate', self.on_add_stock)
		self.glade.get_widget("remove").connect('clicked', self.on_remove_stock)
		self.glade.get_widget("remove").connect('activate', self.on_remove_stock)
		self.treeview.connect('key-press-event', self.on_tree_keypress)
		
		store = gtk.ListStore(str, int, float, float)
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
			
		cell_description = gtk.CellRendererText ()
		cell_description.set_property("editable", True)
		cell_description.connect("edited", on_cell_edited, 0, str)
		column_description = gtk.TreeViewColumn (_("Symbol"), cell_description)
		column_description.set_attributes (cell_description, text=0)
		self.treeview.append_column(column_description)
		
		cell_description = gtk.CellRendererText ()
		cell_description.set_property("editable", True)
		cell_description.connect("edited", on_cell_edited, 1, int)
		column_description = gtk.TreeViewColumn (_("Amount"), cell_description)
		column_description.set_cell_data_func(cell_description, get_cell_data, (int, 1))
		self.treeview.append_column(column_description)
		
		cell_description = gtk.CellRendererText ()
		cell_description.set_property("editable", True)
		cell_description.connect("edited", on_cell_edited, 2, float)
		column_description = gtk.TreeViewColumn (_("Price") + " ($)", cell_description)
		column_description.set_cell_data_func(cell_description, get_cell_data, (float, 2))
		self.treeview.append_column(column_description)
		
		cell_description = gtk.CellRendererText ()
		cell_description.set_property("editable", True)
		cell_description.connect("edited", on_cell_edited, 3, float)
		column_description = gtk.TreeViewColumn (_("Commission") + " ($)", cell_description)
		column_description.set_cell_data_func(cell_description, get_cell_data, (float, 3))
		self.treeview.append_column(column_description)
		
		for key, values in invest.STOCKS.items():
			store.append([key, values["amount"], values["bought"], values["comission"]])

		try:
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-16.png"), -1,-1)
			self.dialog.set_icon(pixbuf)
		except Exception, msg:
			pass
				
		self.sync_ui()
		
	def show_run_hide(self):
		self.dialog.show_all()
		self.dialog.run()
		self.dialog.destroy()
		
		invest.STOCKS = {}
		
		def save_symbol(model, path, iter):
			if int(model[iter][1]) == 0 or float(model[iter][2]) < 0.0001:
				return
				
			invest.STOCKS[model[iter][0]] = {
				"amount": int(model[iter][1]),
				"bought": float(model[iter][2]),
				"comission": float(model[iter][3]),
			}
		self.model.foreach(save_symbol)
		try:
			cPickle.dump(invest.STOCKS, file(invest.STOCKS_FILE, 'w'))
			print 'Stocks written to file'
		except Exception, msg:
			print 'Could not save stocks file:', msg
			
	
	def sync_ui(self):
		pass
	
	def on_add_stock(self, w):
		self.treeview.get_model().append(["GOOG", 10, 400, 30])
		
	def on_remove_stock(self, w):
		model, paths = self.treeview.get_selection().get_selected_rows()
		for path in paths:
			model.remove(model.get_iter(path))
			
	def on_tree_keypress(self, w, event):
		if event.keyval == 65535:
			self.on_remove_stock(self, w)
			
		return False
		
def show_preferences(applet):
	PrefsDialog(applet).show_run_hide()
