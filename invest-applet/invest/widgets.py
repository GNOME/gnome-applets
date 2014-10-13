from gi.repository import Gtk
from gettext import gettext as _
import locale
import invest, invest.about, invest.chart

COLORSCALE_POSITIVE = [
	"white",
	"#ad7fa8",
	"#75507b",
	"#5c3566",
	"#729fcf",
	"#3465a4",
	"#204a87",
	"#8ae234",
	"#73d216",
	"#4e9a06",
]
COLORSCALE_NEGATIVE = [
	"white",
	"#fce94f",
	"#e9b96e",
	"#fcaf3e",
	"#c17d11",
	"#f57900",
	"#ce5c00",
	"#ef2929",
	"#cc0000",
	"#a40000",
]
LIGHT = -3
MEDIUM = -1

TICKER_TIMEOUT = 10000#3*60*1000#

class InvestWidget(Gtk.TreeView):
	def __init__(self, quotes_updater):
		Gtk.TreeView.__init__(self)
		self.set_property("rules-hint", True)
#		self.set_property("enable-grid-lines", True)
#		self.set_property("reorderable", True)
		self.set_property("hover-selection", True)

		simple_quotes_only = quotes_updater.simple_quotes_only(invest.STOCKS)

		# model: SYMBOL, LABEL, TICKER_ONLY, BALANCE, BALANCE_PCT, VALUE, VARIATION_PCT, PB
		# Translators: these words all refer to a stock. Last is short
		# for "last price". Gain is referring to the gain since the
		# stock was purchased.
		col_names = [_('Ticker'), _('Last'), _('Change %'), _('Chart'), _('Gain'), _('Gain %')]
		col_cellgetdata_functions = [self._getcelldata_label, self._getcelldata_value,
			self._getcelldata_variation, None, self._getcelldata_balance, 
			self._getcelldata_balancepct]
		for i, col_name in enumerate(col_names):
			if i < 3:
				cell = Gtk.CellRendererText()
				if i > 0:
					cell.set_property("xalign", 1.0)
				column = Gtk.TreeViewColumn (col_name, cell)
				if i == 0:
					column.set_sort_column_id(quotes_updater.LABEL)
				elif i == 2:
					column.set_sort_column_id(quotes_updater.VARIATION_PCT)
				column.set_cell_data_func(cell, col_cellgetdata_functions[i])
				self.append_column(column)
			elif i == 3:
				if 'hidecharts' in invest.CONFIG and invest.CONFIG['hidecharts']:
					continue
				cell_pb = Gtk.CellRendererPixbuf()
				column = Gtk.TreeViewColumn (col_name, cell_pb, pixbuf=quotes_updater.PB)
				self.append_column(column)
			else:
				# add the last two column only if we have any positions
				if simple_quotes_only == False:
					cell = Gtk.CellRendererText()
					cell.set_property("xalign", 1.0)
					column = Gtk.TreeViewColumn (col_name, cell)
					if i == 4:
						column.set_sort_column_id(quotes_updater.BALANCE)
					elif i == 5:
						column.set_sort_column_id(quotes_updater.BALANCE_PCT)
					column.set_cell_data_func(cell, col_cellgetdata_functions[i])
					self.append_column(column)

		self.connect('row-activated', self.on_row_activated)
		self.set_model(quotes_updater)


	# locale-aware formatting of the value as monetary, without currency symbol, using 2 decimal digits
	def format_currency(self, value, currency):
		return locale.format("%.2f", value, True, True) + " " + currency

	# locale-aware formatting of the percent float (decimal point, thousand grouping point) with 2 decimal digits
	def format_percent(self, value):
		return locale.format("%+.2f", value, True) + "%"

	# locale-aware formatting of the float value (decimal point, thousand grouping point) with sign and 2 decimal digits
	def format_difference(self, value, currency):
		return locale.format("%+.2f", value, True, True) + " " + currency


	def _getcelldata_label(self, column, cell, model, iter, userdata):
		label = model[iter][model.LABEL]
		if self.is_stock(iter):
			cell.set_property('text', label)
		else:
			cell.set_property('markup', "<b>%s</b>" % label)

	def _getcelldata_value(self, column, cell, model, iter, userdata):
		value = model[iter][model.VALUE];
		currency = model[iter][model.CURRENCY];
		if value == None or currency == None:
			cell.set_property('text', "")
		else:
			cell.set_property('text', self.format_currency(value, currency))

	def is_selected(self, model, iter):
		m, it = self.get_selection().get_selected()
		return it != None and model.get_path(iter) == m.get_path(it)

	def get_color(self, model, iter, field):
		palette = COLORSCALE_POSITIVE
		intensity = MEDIUM
		if model[iter][field] < 0:
			palette = COLORSCALE_NEGATIVE
		if self.is_selected(model, iter):
			intensity = LIGHT
		return palette[intensity]

	def _getcelldata_variation(self, column, cell, model, iter, userdata):
		if self.is_group(iter):
			cell.set_property('text', '')
			return

		color = self.get_color(model, iter, model.VARIATION_PCT)
		change_pct = self.format_percent(model[iter][model.VARIATION_PCT])
		cell.set_property('markup',
			"<span foreground='%s'>%s</span>" %
			(color, change_pct))

	def _getcelldata_balance(self, column, cell, model, iter, userdata):
		is_ticker_only = model[iter][model.TICKER_ONLY]
		color = self.get_color(model, iter, model.BALANCE)
		if is_ticker_only:
			cell.set_property('text', '')
		else:
			balance = self.format_difference(model[iter][model.BALANCE], model[iter][model.CURRENCY])
			cell.set_property('markup',
				"<span foreground='%s'>%s</span>" %
				(color, balance))

	def _getcelldata_balancepct(self, column, cell, model, iter, userdata):
		is_ticker_only = model[iter][model.TICKER_ONLY]
		color = self.get_color(model, iter, model.BALANCE_PCT)
		if is_ticker_only:
			cell.set_property('text', '')
		else:
			balance_pct = self.format_percent(model[iter][model.BALANCE_PCT])
			cell.set_property('markup',
				"<span foreground='%s'>%s</span>" %
				(color, balance_pct))

	def on_row_activated(self, treeview, path, view_column):
		ticker = self.get_model()[self.get_model().get_iter(path)][0]
		if ticker == None:
			return

		invest.chart.show_chart([ticker])

	def is_group(self, iter):
		return self.get_model()[iter][0] == None

	def is_stock(self, iter):
		return not self.is_group(iter)
