import os, time
from os.path import *

from gi.repository import GObject, Gtk, Gdk, GdkPixbuf, GConf, PanelApplet
GObject.threads_init()

from gettext import gettext as _

import invest, invest.about, invest.chart, invest.preferences, invest.defs
from invest.quotes import QuoteUpdater
from invest.widgets import *

Gtk.Window.set_default_icon_from_file(join(invest.ART_DATA_DIR, "invest_neutral.svg"))

WidgetBorderWidth = 4
WidgetPaddingSpace = 10

class InvestApplet(PanelApplet.Applet):
	def __init__(self, applet):
		invest.debug("init applet");
		self.applet = applet

		# name, stock_id, label, accellerator, tooltip, callback
		menu_actions = [("About", Gtk.STOCK_HELP, _("About"), None, None, self.on_about),
				("Help", Gtk.STOCK_HELP, _("Help"), None, None, self.on_help),
				("Prefs", Gtk.STOCK_PREFERENCES, _("Preferences"), None, None, self.on_preferences),
				("Refresh", Gtk.STOCK_REFRESH, _("Refresh"), None, None, self.on_refresh)
				]
		actiongroup = Gtk.ActionGroup.new("InvestAppletActions")
		actiongroup.set_translation_domain(invest.defs.GETTEXT_PACKAGE)
		actiongroup.add_actions(menu_actions, None)
		self.applet.setup_menu_from_file (join(invest.defs.PKGDATADIR, "ui/invest-applet-menu.xml"),
						  actiongroup);

		evbox = Gtk.HBox()
		self.applet_icon = Gtk.Image()
		self.set_applet_icon(0)
		self.applet_icon.show()
		evbox.add(self.applet_icon)
		self.applet.add(evbox)
		self.applet.connect("button-press-event", self.button_clicked)
		self.applet.show_all()
		self.new_ilw()

	def new_ilw(self):
		self.quotes_updater = QuoteUpdater(self.set_applet_icon,
						   self.set_applet_tooltip)
		self.investwidget = InvestWidget(self.quotes_updater)
		self.ilw = InvestmentsListWindow(self.applet, self.investwidget)

	def reload_ilw(self):
		self.ilw.destroy()
		self.new_ilw()

	def button_clicked(self, widget,event):
		if event.type == Gdk.EventType.BUTTON_PRESS and event.button == 1:
			# Three cases...
			if len (invest.STOCKS) == 0:
				# a) We aren't configured yet
				invest.preferences.show_preferences(self, _("<b>You have not entered any stock information yet</b>"))
				self.reload_ilw()
			elif not self.quotes_updater.quotes_valid:
				# b) We can't get the data (e.g. offline)
				alert = Gtk.MessageDialog(buttons=Gtk.ButtonsType.CLOSE)
				alert.set_markup(_("<b>No stock quotes are currently available</b>"))
				alert.format_secondary_text(_("The server could not be contacted. The computer is either offline or the servers are down. Try again later."))
				alert.run()
				alert.destroy()
			else:
				# c) Everything is normal: pop-up the window
				self.ilw.toggle_show()
	
	def on_about(self, action, data):
		invest.about.show_about()
	
	def on_help(self, action, data):
		invest.help.show_help()

	def on_preferences(self, action, data):
		invest.preferences.show_preferences(self)
		self.reload_ilw()
	
	def on_refresh(self, action, data):
		self.quotes_updater.refresh()

	def set_applet_icon(self, change):
		if change == 1:
			pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-22_up.png"), -1,-1)
		elif change == 0:
			pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-22_neutral.png"), -1,-1)
		else:
			pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-22_down.png"), -1,-1)
		self.applet_icon.set_from_pixbuf(pixbuf)
	
	def set_applet_tooltip(self, text):
		self.applet_icon.set_tooltip_text(text)

class InvestmentsListWindow(Gtk.Window):
	def __init__(self, applet, list):
		Gtk.Window.__init__(self, type=Gtk.WindowType.TOPLEVEL)
		self.list = list
		self.list.connect('row-collapsed', self.on_collapsed)
		self.list.connect('row-expanded', self.on_expanded)

		self.set_type_hint(Gdk.WindowTypeHint.DOCK)
		self.stick()
		self.set_border_width(WidgetBorderWidth)
		
		self.applet = applet # this is the widget we want to align with
		self.alignment = self.applet.get_orient ()

		sw = Gtk.ScrolledWindow()
		sw.set_policy(Gtk.PolicyType.NEVER,Gtk.PolicyType.AUTOMATIC)
		sw.add(list)
		self.add(sw)

		# boolean variable that identifies if the window is visible
		# show/hide is triggered by left-clicking on the applet
		self.hidden = True

	def toggle_show(self):
		if self.hidden == True:
			self.update_position()
			self.show_all()
			self.hidden = False
		elif self.hidden == False:
			self.hide()
			self.hidden = True

	def on_collapsed(self, treeview, iter, path):
		self.update_position()

	def on_expanded(self, treeview, iter, path):
		self.update_position()

	def update_position (self):
		"""
		Calculates the position and moves the window to it.
		"""
		self.realize()

		# get information sources
		window = self.applet.get_window()
		screen = window.get_screen()
		monitor = screen.get_monitor_geometry (screen.get_monitor_at_window (window))

		# Get applet position and dimensions ...
		(ret, ax, ay) = window.get_origin()
		(ignored, ignored, aw, ah) = window.get_geometry()

		# ... and widget dimensions
		size = self.list.size_request()
		ww = size.width + self.get_border_width() * 2
		wh = size.height + self.get_border_width() * 2

		invest.debug("applet has position (%d,%d) and size (%d,%d)" % (ax, ay, aw, ah))
		invest.debug("applet widget has size (%d,%d)" % (ww, wh))
		invest.debug("monitor has position (%d,%d) and size (%d,%d)" % (monitor.x, monitor.y, monitor.width, monitor.height))
		if self.alignment == PanelApplet.AppletOrient.LEFT:
			invest.debug("applet orientation is LEFT")
			wx = ax - ww - WidgetPaddingSpace
			wy = ay

			if (wy + wh + WidgetPaddingSpace > monitor.y + monitor.height):
				wy = monitor.y + monitor.height - wh - WidgetPaddingSpace

			if (wy + wh + WidgetPaddingSpace > monitor.height):
				wh = wh - (wy + wh + WidgetPaddingSpace - monitor.height)
				wy = monitor.height - wh - WidgetPaddingSpace

			if (wy < WidgetPaddingSpace):
				wh = wh + (wy - WidgetPaddingSpace)
				wy = WidgetPaddingSpace

			if (wy + wh > monitor.height / 2):
				gravity = Gdk.Gravity.SOUTH_WEST
			else:
				gravity = Gdk.Gravity.NORTH_WEST

		elif self.alignment == PanelApplet.AppletOrient.RIGHT:
			invest.debug("applet orientation is RIGHT")
			wx = ax + aw
			wy = ay

			if (wy + wh + WidgetPaddingSpace > monitor.y + monitor.height):
				wy = monitor.y + monitor.height - wh - WidgetPaddingSpace

			if (wy + wh + WidgetPaddingSpace > monitor.height):
				wh = wh - (wy + wh + WidgetPaddingSpace - monitor.height)
				wy = monitor.height - wh - WidgetPaddingSpace

			if (wy < WidgetPaddingSpace):
				wh = wh + (wy - WidgetPaddingSpace)
				wy = WidgetPaddingSpace

			if (wy + wh > monitor.height / 2):
				gravity = Gdk.Gravity.SOUTH_EAST
			else:
				gravity = Gdk.Gravity.NORTH_EAST

		elif self.alignment == PanelApplet.AppletOrient.DOWN:
			invest.debug("applet orientation is DOWN")
			wx = ax
			wy = ay + ah

			if (wx + ww > monitor.x + monitor.width):
				wx = monitor.x + monitor.width - ww
			if (wx < 0):
				wx = 0

			if (wy + wh + WidgetPaddingSpace > monitor.height):
				wh = wh - (wy + wh + WidgetPaddingSpace - monitor.height)
				wy = monitor.height - wh - WidgetPaddingSpace

			gravity = Gdk.Gravity.NORTH_WEST
		elif self.alignment == PanelApplet.AppletOrient.UP:
			invest.debug("applet orientation is UP")
			invest.debug("widget size is (%d,%d)" % (ww, wh))
			wx = ax
			wy = ay - wh

			if (wx + ww > monitor.x + monitor.width):
				wx = monitor.x + monitor.width - ww
			if (wx < 0):
				wx = 0

			invest.debug("current widget position is (%d,%d)" % (wx, wy))
			if (wy < WidgetPaddingSpace):
				wy = WidgetPaddingSpace
			if (wy + wh + WidgetPaddingSpace > monitor.height):
				wh = monitor.height - wy - WidgetPaddingSpace


			gravity = Gdk.Gravity.SOUTH_WEST

		invest.debug("Set widget position to (%d,%d)" % (wx, wy))
		self.move(wx, wy)
		invest.debug("Set widget size to (%d,%d)" % (ww, wh))
		self.resize(ww, wh)
		invest.debug("Set widget gravity to %s" % gravity)
		self.set_gravity(gravity)
