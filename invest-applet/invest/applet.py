import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gobject
from gettext import gettext as _

import invest, invest.about, invest.chart, invest.preferences
from invest.quotes import get_quotes_updater
from invest.widgets import *

		
class InvestAppletPreferences:
	def __init__(self, applet):
		# Default values
		self.GCONF_APPLET_DIR = invest.GCONF_DIR
		
		# Retreive this applet's pref folder
		path = applet.get_preferences_key()
		if path != None:
			self.GCONF_APPLET_DIR = path			
			print 'Using per-applet gconf key:', self.GCONF_APPLET_DIR
						
class InvestApplet:
	def __init__(self, applet):
		self.applet = applet
		self.prefs = InvestAppletPreferences(applet)
		
		self.investwidget = InvestWidget()
		self.investticker = InvestTicker()
		
		self.pw = ProgramWindow(applet, self.investwidget)
		self.tb = ToggleButton(applet, self.pw)
		
		self.investwidget.connect('row-activated', lambda treeview, path, view_column: self.tb.set_active(False))
		get_quotes_updater().connect('quotes-updated', self._on_quotes_updated)
			
		box = gtk.HBox()
		box.add(self.tb)
		box.add(self.investticker)
		
		self.applet.add(box)
		self.applet.setup_menu_from_file (
			invest.SHARED_DATA_DIR, "Invest_Applet.xml",
			None, [("About", self.on_about), ("Prefs", self.on_preferences), ("Refresh", self.on_refresh)])

		self.applet.show_all()
		get_quotes_updater().refresh()
		
	def on_about(self, component, verb):
		invest.about.show_about()
	
	def on_preferences(self, component, verb):
		invest.preferences.show_preferences(self)
		get_quotes_updater().refresh()
	
	def on_refresh(self, component, verb):
		get_quotes_updater().refresh()
		
	def _on_quotes_updated(self, updater):
		pass
		#invest.dbusnotification.notify(
		#	_("Financial Report"),
		#	"stock_chart",
		#	_("Financial Report"),
		#	_("Quotes updated"),
		#	3000)
			
class ToggleButton(gtk.ToggleButton):
	def __init__(self, applet, pw):
		gtk.ToggleButton.__init__(self)
		self.pw = pw

		self.connect("toggled", self.toggled)

		self.set_relief(gtk.RELIEF_NONE)

		image = gtk.Image()
		try:
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-button.png"), -1,-1)
			image.set_from_pixbuf(pixbuf)
		except Exception, msg:
			image.set_from_icon_name("stock_chart", gtk.ICON_SIZE_BUTTON)
		
		hbox = gtk.HBox()
		hbox.pack_start(image)
		
		orient = applet.get_orient()
		arrow_dir = gtk.ARROW_DOWN
		if orient == gnomeapplet.ORIENT_RIGHT:
			arrow_dir = gtk.ARROW_RIGHT
		elif orient == gnomeapplet.ORIENT_LEFT:
			arrow_dir = gtk.ARROW_LEFT
		elif orient == gnomeapplet.ORIENT_DOWN:
			arrow_dir = gtk.ARROW_DOWN
		elif orient == gnomeapplet.ORIENT_UP:
			arrow_dir = gtk.ARROW_UP
			
		hbox.pack_start(gtk.Arrow(arrow_dir, gtk.SHADOW_NONE))
		
		self.add(hbox)
			
	def toggled(self, togglebutton):
		if togglebutton.get_active():
			self.pw.position_window(self)
			self.pw.show_all()
			self.pw.grab_focus()
		else:
			self.pw.hide()

gobject.type_register(ToggleButton)
	
class ProgramWindow(gtk.Window):
	def __init__(self, applet, widget):
		gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
		self.applet = applet
		
		self.set_decorated(False)
		self.set_resizable(False)
		
		frame = gtk.Frame()
		frame.add(widget)
		frame.set_shadow_type(gtk.SHADOW_IN)

		self.add(frame)

	def position_window(self, widget):
		self.realize()
		gtk.gdk.flush()

		self.stick()
		self.set_keep_above(True)
		self.set_skip_pager_hint(True)
		self.set_skip_taskbar_hint(True)

		widget.realize()
		(x, y) = widget.window.get_origin()

		(w, h) = self.get_size()
		(w, h) = self.size_request()

		button_w = widget.allocation.width
		button_h = widget.allocation.height

		screen = self.get_screen()

		found_monitor = False
		n = screen.get_n_monitors()
		for i in range(0, n):
			monitor = screen.get_monitor_geometry(i)
			if (x >= monitor.x and x <= monitor.x + monitor.width and
				y >= monitor.y and y <= monitor.y + monitor.height):
					found_monitor = True
					break

		if not found_monitor:
			screen_width = screen.get_width()
			monitor = gtk.gdk.Rectangle(0, 0, screen_width, screen_width)

		orient = self.applet.get_orient()

		if orient == gnomeapplet.ORIENT_RIGHT:
			x += button_w

			if ((y + h) > monitor.y + monitor.height):
				y -= (y + h) - (monitor.y + monitor.height)

			if ((y + h) > (monitor.height / 2)):
				gravity = gtk.gdk.GRAVITY_SOUTH_WEST
			else:
				gravity = gtk.gdk.GRAVITY_NORTH_WEST
		elif orient == gnomeapplet.ORIENT_LEFT:
			x -= w

			if ((y + h) > monitor.y + monitor.height):
				y -= (y + h) - (monitor.y + monitor.height)

			if ((y + h) > (monitor.height / 2)):
				gravity = gtk.gdk.GRAVITY_SOUTH_EAST
			else:
				gravity = gtk.gdk.GRAVITY_NORTH_EAST
		elif orient == gnomeapplet.ORIENT_DOWN:
			y += button_h

			if ((x + w) > monitor.x + monitor.width):
				x -= (x + w) - (monitor.x + monitor.width)

			gravity = gtk.gdk.GRAVITY_NORTH_WEST
		elif orient == gnomeapplet.ORIENT_UP:
			y -= h

			if ((x + w) > monitor.x + monitor.width):
				x -= (x + w) - (monitor.x + monitor.width)

			gravity = gtk.gdk.GRAVITY_SOUTH_WEST

		self.move(x, y)
		self.set_gravity(gravity)
		self.show()

gobject.type_register(ProgramWindow)
