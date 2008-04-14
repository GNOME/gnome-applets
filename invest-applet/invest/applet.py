import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gobject
from gettext import gettext as _
import gconf

import invest, invest.about, invest.chart, invest.preferences
from invest.quotes import QuoteUpdater
from invest.widgets import *

class InvestApplet:
	def __init__(self, applet):
		self.applet = applet
		self.applet.setup_menu_from_file (
			None, "Invest_Applet.xml",
			None, [("About", self.on_about), 
					("Prefs", self.on_preferences),
					("Refresh", self.on_refresh)
					])

		evbox = gtk.HBox()
		applet_icon = gtk.Image()
		try:
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-button.png"), -1,-1)
			applet_icon.set_from_pixbuf(pixbuf)
		except Exception, msg:
			applet_icon.set_from_icon_name("stock_chart-autoformat", gtk.ICON_SIZE_BUTTON)
	
		applet_icon.show()
		evbox.add(applet_icon)
		self.investments_change_icon = gtk.Image()
		evbox.add(self.investments_change_icon)
                self.set_investments_change_icon(0)
		self.applet.add(evbox)
		self.applet.connect("button-press-event",self.button_clicked)
		self.applet.show_all()
		self.new_ilw()

	def new_ilw(self):
		self.quotes_updater = QuoteUpdater(self.set_investments_change_icon)
		self.investwidget = InvestWidget(self.quotes_updater)
		self.ilw = InvestmentsListWindow(self.applet, self.investwidget)

	def button_clicked(self, widget,event):
		if event.type == gtk.gdk.BUTTON_PRESS and event.button == 1:
			self.ilw.toggle_show()
	
	def on_about(self, component, verb):
		invest.about.show_about()
	
	def on_preferences(self, component, verb):
		invest.preferences.show_preferences(self)
		self.ilw.destroy()
		self.new_ilw()
	
	def on_refresh(self, component, verb):
		self.quotes_updater.refresh()

	def set_investments_change_icon(self, change):
		if change == 1:
			self.investments_change_icon.set_from_stock(gtk.STOCK_GO_UP, gtk.ICON_SIZE_MENU)
		elif change == 0:
                        self.investments_change_icon.set_from_stock(gtk.STOCK_REMOVE, gtk.ICON_SIZE_MENU)
		else:
                        self.investments_change_icon.set_from_stock(gtk.STOCK_GO_DOWN, gtk.ICON_SIZE_MENU)

class InvestmentsListWindow(gtk.Window):
	def __init__(self, applet, list):
		gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
		self.set_type_hint(gtk.gdk.WINDOW_TYPE_HINT_DOCK)
		self.stick()
		self.set_resizable(False)
		
		self.applet = applet # this is the widget we want to align with
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
		self.realize()

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
