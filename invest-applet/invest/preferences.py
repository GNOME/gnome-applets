from gettext import gettext as _
import locale
from os.path import join
from gi.repository import GObject, Gtk, GConf
import invest
import currencies
import cPickle

class PrefsDialog:
	def __init__(self, applet):
		self.ui = Gtk.Builder()
		self.ui.add_from_file(join(invest.BUILDER_DATA_DIR, "prefs-dialog.ui"))

		self.dialog = self.ui.get_object("preferences")
		self.treeview = self.ui.get_object("stocks")
		self.currency = self.ui.get_object("currency")
		self.currency_code = None
		self.currencies = currencies.Currencies.currencies
		self.indexexpansion = self.ui.get_object("indexexpansion")
		if invest.CONFIG.has_key('indexexpansion'):
			self.indexexpansion.set_active(invest.CONFIG['indexexpansion'])
		else:
			self.indexexpansion.set_active(False)

		self.hidecharts = self.ui.get_object("hidecharts")
		if invest.CONFIG.has_key('hidecharts'):
			self.hidecharts.set_active(invest.CONFIG['hidecharts'])
		else:
			self.hidecharts.set_active(False)

		self.ui.get_object("addstock").connect('clicked', self.on_add, False)
		self.ui.get_object("addstock").connect('activate', self.on_add, False)
		self.ui.get_object("addgroup").connect('clicked', self.on_add, True)
		self.ui.get_object("addgroup").connect('activate', self.on_add, True)
		self.ui.get_object("remove").connect('clicked', self.on_remove_stock)
		self.ui.get_object("remove").connect('activate', self.on_remove_stock)
		self.ui.get_object("help").connect('clicked', self.on_help)
		self.treeview.connect('key-press-event', self.on_tree_keypress)
		self.currency.connect('key-press-event', self.on_entry_keypress)
		self.currency.connect('activate', self.on_activate_entry)
		self.currency.connect('focus-out-event', self.on_focus_out_entry)

		self.typs = (str, str, float, float, float, float)
		self.names = (_("Symbol"), _("Label"), _("Amount"), _("Price"), _("Commission"), _("Currency Rate"))
		store = Gtk.TreeStore(*self.typs)
		store.set_sort_column_id(0, Gtk.SortType.ASCENDING)
		self.treeview.set_model(store)
		self.model = store

		completion = Gtk.EntryCompletion()
		self.currency.set_completion(completion)
		liststore = Gtk.ListStore(GObject.TYPE_STRING, GObject.TYPE_STRING)
		completion.set_model(liststore)
		completion.set_text_column(0)
		for code, label in self.currencies.items():
			liststore.append([self.format_currency(label, code), code])
		completion.set_match_func(self.match_func, 0)
		completion.connect("match-selected", self.on_completion_selection, 1)

		if invest.CONFIG.has_key("currency"):
			code = invest.CONFIG["currency"];
			if self.currencies.has_key(code):
				self.currency_code = code;
				currency = self.format_currency(self.currencies[self.currency_code], self.currency_code)
				self.currency.set_text(currency)

		for n in xrange (0, 5):
			self.create_cell (self.treeview, n, self.names[n], self.typs[n])
		if self.currency_code != None:
			self.add_exchange_column()

		self.add_to_store(store, None, invest.STOCKS)

		self.sync_ui()

	def is_group(self, iter):
		return self.model[iter][1] == None

	def is_stock(self, iter):
		return not self.is_group(iter)

	def on_cell_edited(self, cell, path, new_text, col, typ):
		if col != 0 and self.is_group(self.model.get_iter(path)):
			return

		try:
			if col == 0 and self.is_stock(self.model.get_iter(path)):    # stock symbols must be uppercase
				new_text = str.upper(new_text)
			if col < 2:
				self.model[path][col] = new_text
			else:
				value = locale.atof(new_text)
				self.model[path][col] = value
		except Exception, msg:
			invest.error('Exception while processing cell change: %s' % msg)
			pass

	def format(self, fmt, value):
		return locale.format(fmt, value, True)

	def get_cell_data(self, column, cell, model, iter, data):
		typ, col = data
		if self.is_group(iter):
			if col == 0:
				val = model[iter][col]
				cell.set_property('markup', "<b>%s</b>" % typ(val))
			else:
				cell.set_property('text', "")
			return

		val = model[iter][col]
		if typ == int:
			cell.set_property('text', "%d" % typ(val))
		elif typ == float:
			# provide float numbers with at least 2 fractional digits
			digits = self.fraction_digits(val)
			fmt = "%%.%df" % max(digits, 2)
			cell.set_property('text', self.format(fmt, val))
		else:
			cell.set_property('text', typ(val))

	# determine the number of non zero digits in the fraction of the value
	def fraction_digits(self, value):
		text = "%g" % value	# do not use locale here, so that %g always is rendered to a number with . as decimal separator
		if text.find(".") < 0:
			return 0
		return len(text) - text.find(".") - 1

	def create_cell (self, view, column, name, typ):
		cell_description = Gtk.CellRendererText ()
		if typ == float:
			cell_description.set_property("xalign", 1.0)
		cell_description.set_property("editable", True)
		cell_description.connect("edited", self.on_cell_edited, column, typ)
		column_description = Gtk.TreeViewColumn (name, cell_description)
		if typ == str:
			column_description.set_sort_column_id(column)
		column_description.set_cell_data_func(cell_description, self.get_cell_data, (typ, column))
		view.append_column(column_description)

	def add_exchange_column(self):
		self.create_cell (self.treeview, 5, self.names[5], self.typs[5])

	def remove_exchange_column(self):
		column = self.treeview.get_column(5)
		self.treeview.remove_column(column)

	def show_run_hide(self, explanation = ""):
		expl = self.ui.get_object("explanation")
		expl.set_markup(explanation)
		self.dialog.show_all()
		if explanation == "":
			expl.hide()
		# returns 1 if help is clicked
		while self.dialog.run() == 1:
			pass
		self.dialog.destroy()


		# transform the stocks treestore into the STOCKS list
		invest.STOCKS = self.to_list(self.model.get_iter_first())

		# store the STOCKS into the pickles file
		try:
			cPickle.dump(invest.STOCKS, file(invest.STOCKS_FILE, 'w'))
			invest.debug('Stocks written to file')
		except Exception, msg:
			invest.error('Could not save stocks file: %s' % msg)

		# store the CONFIG (currency, index expansion) into the config file
		invest.CONFIG = {}
		if self.currency_code != None and len(self.currency_code) == 3:
			invest.CONFIG['currency'] = self.currency_code
		invest.CONFIG['indexexpansion'] = self.indexexpansion.get_active()
		invest.CONFIG['hidecharts'] = self.hidecharts.get_active()
		try:
			cPickle.dump(invest.CONFIG, file(invest.CONFIG_FILE, 'w'))
			invest.debug('Configuration written to file')
		except Exception, msg:
			invest.debug('Could not save configuration file: %s' % msg)


	def to_list(self, iter):
		stocks = {}
		groups = []

		while iter != None:
			if self.is_stock(iter):
				ticker = self.model[iter][0]
				if not ticker in stocks:
					stocks[ticker] = {
								'ticker': ticker,
								'label': self.model[iter][1],
								'purchases': []
							 }
				stocks[ticker]["purchases"].append({
					"amount": float(self.model[iter][2]),
					"bought": float(self.model[iter][3]),
					"comission": float(self.model[iter][4]),
					"exchange": float(self.model[iter][5])
				})

			else:
				name = self.model[iter][0]
				list = self.to_list(self.model.iter_children(iter))
				groups.append({ 'name': name, 'list': list })

			iter = self.model.iter_next(iter)

		groups.extend(stocks.values())
		return groups

	def add_to_store(self, store, parent, stocks):
		for stock in stocks:
			if not stock.has_key('ticker'):
				name = stock['name']
				list = stock['list']
				row = store.append(parent, [name, None, None, None, None, None])
				self.add_to_store(store, row, list)
			else:
				ticker = stock['ticker']
				label = stock["label"]
				purchases = stock["purchases"]
				for purchase in purchases:
					if purchase.has_key("exchange"):
						exchange =  purchase["exchange"]
					else:
						exchange = 0.0
					store.append(parent, [ticker, label, float(purchase["amount"]), float(purchase["bought"]), float(purchase["comission"]), float(exchange)])

	def sync_ui(self):
		pass

	def on_add(self, w, group = False):
		# get the selected row
		(model, parent) = self.treeview.get_selection().get_selected()

		# if the selected row is a stock, move to its parent
		while parent != None and self.is_stock(parent):
			parent = model.iter_parent(parent)

		# add a new row as child of the selected row
		if group:
			iter = self.model.append(parent, [_("Stock Group"), None, None, None, None, None])
			# select the group and add a stock
			path = self.model.get_path(iter)
			self.treeview.set_cursor(path, None, False)
			self.on_add(w, False)
			# make sure the new group is elapsed
			self.treeview.expand_row(path, False)
			# select the new group for editing
		else:
			iter = self.model.append(parent, ["YHOO", "Yahoo! Inc.", 0.0, 0.0, 0.0, 0.0])
			# make sure the new stock's parent is elapsed
			if parent != None:
				path = self.model.get_path(parent)
				self.treeview.expand_row(path, False)
			# select the new row for editing
			path = self.model.get_path(iter)
		self.treeview.set_cursor(path, self.treeview.get_column(0), True)


	def on_remove_stock(self, w):
		model, paths = self.treeview.get_selection().get_selected_rows()
		for path in paths:
			iter = model.get_iter(path)
			if model.iter_n_children(iter) > 0:
				# translators: Asks the user to confirm deletion of a group of stocks
				dialog = Gtk.Dialog(_("Delete entire stock group?"),
						None,
						Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
						(Gtk.STOCK_CANCEL, Gtk.ResponseType.REJECT,
						 Gtk.STOCK_OK, Gtk.ResponseType.ACCEPT))
				# translators: stocks can be grouped together into a "stock group".
				# The user wants to delete a group, but the group still contains stocks.
				# By deleting the group, also all stocks will be removed from configuration
				label = Gtk.Label(_("This removes all stocks contained in this stock group!\nDo you really want to remove this stock group?"))
				dialog.get_content_area().pack_start(label, True, True, 10)
				label.show()
				response = dialog.run()
				dialog.destroy()
				if response == Gtk.ResponseType.REJECT:
					continue

			model.remove(iter)

	def on_help(self, w):
		invest.help.show_help_section("invest-applet-usage")

	def on_tree_keypress(self, w, event):
		if event.keyval == 65535:
			self.on_remove_stock(w)

		return False

	def format_currency(self, label, code):
		if code == None:
			return label
		return label + " (" + code + ")"

	def on_entry_keypress(self, w, event):
		# enter key was pressed
		if event.keyval == 65293:
			self.match_currency()
		return False

	# entry was activated (Enter)
	def on_activate_entry(self, entry):
		self.match_currency()

	# entry left focus
	def on_focus_out_entry(self, w, event):
		self.match_currency()
		return False

	# tries to find a currency for the text in the currency entry
	def match_currency(self):
		# get the text
		text = self.currency.get_text().upper()

		# if there is none, finish
		if len(text) == 0:
			self.currency_code = None
			self.pick_currency(None)
			return

		# if it is a currency code, take that one
		if len(text) == 3:
			# try to find the string as code
			if self.currencies.has_key(text):
				self.pick_currency(text)
				return
		else:
			# try to find the code for the string
			for code, label in self.currencies.items():
				# if the entry equals to the full label,
				# or the entry equals to the label+code concat
				if label.upper() == text or self.format_currency(label.upper(), code) == text:
					# then we take that code
					self.pick_currency(code)
					return

		# the entry is not valid, reuse the old one
		self.pick_currency(self.currency_code)

	# pick this currency, stores it and sets the entry text
	def pick_currency(self, code):
		if code == None:
			label = ""
			if len(self.treeview.get_columns()) == 6:
				self.remove_exchange_column()
		else:
			label = self.currencies[code]
			if len(self.treeview.get_columns()) == 5:
				self.add_exchange_column()
		self.currency.set_text(self.format_currency(label, code))
		self.currency_code = code

	# finds matches by testing candidate strings to have tokens starting with the entered text's tokens
	def match_func(self, completion, key, iter, column):
		keys = key.split()
		model = completion.get_model()
		text = model.get_value(iter, column).lower()
		tokens = text.split()

		# each key must have a match
		for key in keys:
			found_key = False
			# check any token of the completions start with the key
			for token in tokens:
				# remove the ( from the currency code
				if token.startswith("("):
					token = token[1:]
				if token.startswith(key):
					found_key = True
					break
			# this key does not have a match, this is not a completion
			if not found_key:
				return False
		# all keys matched, this is a completion
		return True

	# stores the selected currency code
	def on_completion_selection(self, completion, model, iter, column):
		self.pick_currency(model.get_value(iter, column))


def show_preferences(applet, explanation = ""):
	PrefsDialog(applet).show_run_hide(explanation)
