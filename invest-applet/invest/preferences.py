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

		self.typs = (str, str, float, float, float)
		self.names = (_("Symbol"), _("Label"), _("Amount"), _("Price"), _("Commission"))
		store = gtk.ListStore(*self.typs)
		store.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.treeview.set_model(store)
		self.model = store

		def on_cell_edited(cell, path, new_text, col, typ):
			try:
				if col == 0:    # stock symbols must be uppercase
					new_text = str.upper(new_text)
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
				column_description.set_attributes (cell_description, text=column)
				column_description.set_sort_column_id(column)
			if typ == float:
				column_description.set_cell_data_func(cell_description, get_cell_data, (float, column))
			view.append_column(column_description)


		for n in xrange (0, 5):
			create_cell (self.treeview, n, self.names[n], self.typs[n])		
		stock_items = invest.STOCKS.items ()
		stock_items.sort ()
		for key, data in stock_items:
			label = data["label"]
			purchases = data["purchases"]
			for purchase in purchases:
				store.append([key, label, purchase["amount"], purchase["bought"], purchase["comission"]])

		try:
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-16.png"), -1,-1)
			self.dialog.set_icon(pixbuf)
		except Exception, msg:
			invest.debug("Could not load 'invest-16.png' file: %s" % msg)
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
				invest.STOCKS[model[iter][0]] = { 'label': model[iter][1], 'purchases': [] }
				
			invest.STOCKS[model[iter][0]]["purchases"].append({
				"amount": float(model[iter][2]),
				"bought": float(model[iter][3]),
				"comission": float(model[iter][4]),
			})
		self.model.foreach(save_symbol)
		try:
			cPickle.dump(invest.STOCKS, file(invest.STOCKS_FILE, 'w'))
			invest.debug('Stocks written to file')
		except Exception, msg:
			invest.error('Could not save stocks file: %s' % msg)


	def sync_ui(self):
		pass

	def on_add_stock(self, w):
		iter = self.model.append(["GOOG", "Google Inc.", 0, 0, 0])
		path = self.model.get_path(iter)
		self.treeview.set_cursor(path, self.treeview.get_column(0), True)

	def on_remove_stock(self, w):
		model, paths = self.treeview.get_selection().get_selected_rows()
		for path in paths:
			model.remove(model.get_iter(path))

	def on_tree_keypress(self, w, event):
		if event.keyval == 65535:
			self.on_remove_stock(w)

		return False

def show_preferences(applet, explanation = ""):
	PrefsDialog(applet).show_run_hide(explanation)
