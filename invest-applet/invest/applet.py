import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gobject
from gettext import gettext as _
import gconf

import invest, invest.about, invest.chart, invest.preferences
from invest.quotes import get_quotes_updater
from invest.widgets import *

		
class InvestAppletPreferences:
	def __init__(self, applet):
		# Default values
		self.GCONF_APPLET_DIR = invest.GCONF_DIR
		self.GCONF_CLIENT = gconf.client_get_default ()
		
		# Retrieve this applet's pref folder
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

		get_quotes_updater().connect('quotes-updated', self._on_quotes_updated)

		self.applet.setup_menu_from_file (
			invest.SHARED_DATA_DIR, "Invest_Applet.xml",
			None, [("About", self.on_about), ("Prefs", self.on_preferences), ("Refresh", self.on_refresh)])

		evbox = gtk.HBox()
		applet_icon = gtk.Image()
		try:
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-button.png"), -1,-1)
			applet_icon.set_from_pixbuf(pixbuf)
		except Exception, msg:
			applet_icon.set_from_icon_name("stock_chart-autoformat", gtk.ICON_SIZE_BUTTON)
		
		applet_icon.show()
		evbox.add(applet_icon)

		self.ilw = InvestmentsListWindow(self.applet, self.investwidget)

		self.applet.add(evbox)
		self.applet.connect("button-press-event",self.button_clicked)
		self.applet.show_all()
		get_quotes_updater().refresh()

	def button_clicked(self, widget,event):
		if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
			self.ilw.toggle_show()
	
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

class InvestmentsListWindow(gtk.Window):
	def __init__(self, applet, list):
		gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
		self.set_type_hint(gtk.gdk.WINDOW_TYPE_HINT_DOCK)
		self.stick()
		
		self.applet = applet
		self.alignment = self.applet.get_orient ()
		
		self.add(list)
		list.show()

		# boolean variable that identifies if the window is visible
		# show/hide is triggered by left-clicking on the applet
		self.hidden = True

	def toggle_show(self):
		if self.hidden == True:
			self.update_position()
			self.show()
			self.hidden = False
		elif self.hidden == False:
			self.hide()
			self.hidden = True

	def update_position (self):
		"""
		Calculates the position and moves the window to it.
		"""

		# Get our own dimensions & position
		#(wx, wy) = self.get_origin()
		(ax, ay) = self.applet.window.get_origin()

		(ww, wh) = self.get_size ()
		(aw, ah) = self.applet.window.get_size ()

		screen = self.applet.window.get_screen()
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
