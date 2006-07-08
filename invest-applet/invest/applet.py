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
			gtik_settings = self.GCONF_CLIENT.get_string(self.GCONF_APPLET_DIR+"/prefs/tik_syms")
			if gtik_settings != None and gtik_settings != "":
				# Import old settings
				self.GCONF_CLIENT.set_string(self.GCONF_APPLET_DIR+"/prefs/tik_syms", "")
				for sym in gtik_settings.split('+'):
					invest.STOCKS[sym].append({
						"amount": 0,
						"bought": 0,
						"comission": 0,
					})
						
class InvestApplet:
	def __init__(self, applet):
		self.applet = applet
		self.prefs = InvestAppletPreferences(applet)
		
		self.investwidget = InvestWidget()
		self.investticker = InvestTrend()#InvestTicker()
		
		self.pw = ProgramWindow(applet, self.investticker)
		self.pw.add(self.investwidget)
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
			self.pw.update_position()
			self.pw.show_all()
			self.pw.grab_focus()
		else:
			self.pw.hide()

gobject.type_register(ToggleButton)
	
class ProgramWindow(gtk.Window):
	"""
	Borderless window aligning itself to a given widget.
	Use CuemiacWindow.update_position() to align it.
	"""
	def __init__(self, applet, widgetToAlignWith):
		"""
		alignment should be one of
			gnomeapplet.ORIENT_{DOWN,UP,LEFT,RIGHT}
		
		Call CuemiacWindow.update_position () to position the window.
		"""
		gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
		self.set_decorated (False)

		# Skip the taskbar, and the pager, stick and stay on top
		self.stick()
		self.set_keep_above(True)
		self.set_skip_pager_hint(True)
		self.set_skip_taskbar_hint(True)
		
		self.widgetToAlignWith = widgetToAlignWith		
		self.applet = applet
		self.alignment = applet.get_orient ()

		self.realize_status = None
		self.connect ("realize", lambda win : self.__register_realize ())
		self.connect ("size-allocate", self.__resize_event)
		
	def __resize_event (self, event, data):
		self.update_position ()
	
	def update_position (self):
		"""
		Calculates the position and moves the window to it.
		IMPORATNT: widgetToAlignWith should be realized!
		"""
		if self.realize_status == None:
			self.realize_status = False
			self.realize ()
			return
		
		if self.realize_status == False:
			return
			
		# Get our own dimensions & position
		(wx, wy) = self.window.get_origin ()
		(ax, ay) = self.widgetToAlignWith.window.get_origin ()

		(ww, wh) = self.window.get_size ()
		(aw, ah) = self.widgetToAlignWith.window.get_size ()

		screen = self.get_screen()
		monitor = screen.get_monitor_geometry (screen.get_monitor_at_window (self.applet.window))
		
		if self.alignment == gnomeapplet.ORIENT_LEFT:
				x = ax - ww
				y = ay

				if (y + wh > monitor.y + monitor.height):
					y = monitor.y + monitor.height - wh

				if (y < 0):
					y = 0
				
				if (y + wh > monitor.height / 2):
					gravity = gtk.gdk.GRAVITY_SOUTH_WEST	
				else:
					gravity = gtk.gdk.GRAVITY_NORTH_WEST
					
		elif self.alignment == gnomeapplet.ORIENT_RIGHT:
				x = ax + aw
				y = ay

				if (y + wh > monitor.y + monitor.height):
					y = monitor.y + monitor.height - wh
				
				if (y < 0):
					y = 0
				
				if (y + wh > monitor.height / 2):
					gravity = gtk.gdk.GRAVITY_SOUTH_EAST
				else:
					gravity = gtk.gdk.GRAVITY_NORTH_EAST

		elif self.alignment == gnomeapplet.ORIENT_DOWN:
				x = ax
				y = ay + ah

				if (x + ww > monitor.x + monitor.width):
					x = monitor.x + monitor.width - ww

				if (x < 0):
					x = 0

				gravity = gtk.gdk.GRAVITY_NORTH_WEST
		elif self.alignment == gnomeapplet.ORIENT_UP:
				x = ax
				y = ay - wh

				if (x + ww > monitor.x + monitor.width):
					x = monitor.x + monitor.width - ww

				if (x < 0):
					x = 0

				gravity = gtk.gdk.GRAVITY_SOUTH_WEST
		
		self.move(x, y)
		self.set_gravity(gravity)
	
	def __register_realize (self):
		self.realize_status = True
		self.update_position()
		
gobject.type_register (ProgramWindow)
