#!/usr/bin/env python

import gtk, gtk.gdk, gtk.glade
import gobject
import egg.trayicon
import gnomevfs
import os
import invest
from gettext import gettext as _
from invest import *
import sys
from os.path import join

# p:
#  eX = Exponential Moving Average
#  mX = Moving Average
#  b = Bollinger Bands Overlay
#  v = Volume Overlay
#  p = Parabolic SAR overlay
#  s = Splits Overlay
# q:
#  l = Line
#  c = Candles
#  b = Bars
# l:
#  on = Logarithmic
#  off = Linear
# z:
#  l = Large
#  m = Medium
# t:
#  Xd = X Days
#  Xm = X Months
#  Xy = X Years
# a:
#  fX = MFI X days
#  ss = Slow Stochastic
#  fs = Fast Stochastic
#  wX = W%R X Days
#  mX-Y-Z = MACD X Days, Y Days, Signal
#  pX = ROC X Days
#  rX = RSI X Days
#  v = Volume
#  vm = Volume +MA
# c:
#  X = compare with X
#

class FinancialChart:
	def __init__(self, ui):
		self.ui = ui
		
		# Window Properties
		win = ui.get_widget("window")
		win.set_title(_("Financial Chart"))
		
		try:
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest-16.png"), -1,-1)
			win.set_icon(pixbuf)
			pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest.svg"), 96,96)
			self.ui.get_widget("plot").set_from_pixbuf(pixbuf)
		except Exception, msg:
			pass
		
		# Defaut comboboxes values
		for widget in ["t", "q"]:
			ui.get_widget(widget).set_active(0)
		
		# Connect every option widget to its corresponding change signal	
		symbolentry = ui.get_widget("s")
		refresh_chart_callback = lambda w: self.on_refresh_chart()
		
		for widgets, signal in [
			(("pm5","pm10","pm20","pm50","pm100","pm200",
			"pe5","pe10", "pe20","pe50","pe100","pe200",
			"pb","pp","ps","pv",
			"ar","af","ap","aw","am","ass","afs","av","avm"), "toggled"),
			(("t", "q"), "changed"),
			(("s",), "activate"),
		]:
			for widget in widgets:
				ui.get_widget(widget).connect(signal, refresh_chart_callback)
		
		ui.get_widget("progress").hide()
		
		# Connect auto-refresh widget
		self.autorefresh_id = 0
		ui.get_widget("autorefresh").connect("toggled", self.on_autorefresh_toggled)
										
	def on_refresh_chart(self, from_timer=False):
		tickers = self.ui.get_widget("s").get_text()
			
		if tickers.strip() == "":
			return True
		
		if from_timer and not ustime.hour_between(9, 16):
			return True
			
		tickers = [ticker.strip().upper() for ticker in tickers.split(' ') if ticker != ""]
		
		# Update Window Title ------------------------------------------------------
		win = self.ui.get_widget("window")
		title = _("Financial Chart - %s")
		titletail = ""
		for ticker in tickers:
			titletail += "%s / " % ticker
		title = title % titletail
		
		win.set_title(title[:-3])

		# Detect Comparison or simple chart ----------------------------------------
		opt = ""
		for ticker in tickers[1:]:
			opt += "&c=%s" % ticker
		
		# Create the overlay string ------------------------------------------------
		p = ""
		for name, param in [
			("pm5",   5),
			("pm10",  10),
			("pm20",  20),
			("pm50",  50),
			("pm100", 100),
			("pm200", 200),
			("pe5",   5),
			("pe10",  10),
			("pe20",  20),
			("pe50",  50),
			("pe100", 100),
			("pe200", 200),
			("pb", ""),
			("pp", ""),
			("ps", ""),
			("pv", ""),
		]:
			if self.ui.get_widget(name).get_active():
				p += "%s%s," % (name[1], param)
			
		# Create the indicators string ---------------------------------------------
		a = ""
		for name, param in [
			("ar",  14),
			("af",  14),
			("ap",  12),
			("aw",  14),
			("am",  "26-12-9"),
			("ass", ""),
			("afs", ""),
			("av",  ""),
			("avm", ""),
		]:
			if self.ui.get_widget(name).get_active():
				a += "%s%s," % (name[1:], param)
		
		# Create the image URL -----------------------------------------------------
		url = CHART_BASE_URL % {
			"s": tickers[0],
			"t": self.ui.get_widget("t").get_active_text(),
			"q": self.ui.get_widget("q").get_active_text(),
			"l": "off",
			"z": "l",
			"p": p,
			"a": a,
			"opt": opt,
		}
		
		# Download and display the image -------------------------------------------	
		progress = self.ui.get_widget("progress")
		progress.set_text(_("Opening Chart"))
		progress.show()
		
		gnomevfs.async.open(url, lambda h,e: self.on_chart_open(h,e,url))

		# Update timer if needed
		self.on_autorefresh_toggled(self.ui.get_widget("autorefresh"))
				
		return True
	
	def on_chart_open(self, handle, exc_type, url):
		progress = self.ui.get_widget("progress")
		progress.set_text(_("Downloading Chart"))
		
		if not exc_type:
			loader = gtk.gdk.PixbufLoader()
			handle.read(GNOMEVFS_CHUNK_SIZE, self.on_chart_read, (loader, url))
		else:
			handle.close(lambda *args: None)
			progress.set_text("")

	def on_chart_read(self, handle, data, exc_type, bytes_requested, udata):
		loader, url = udata
		progress = self.ui.get_widget("progress")
		progress.set_text(_("Reading Chart chunk"))
		
		if not exc_type:
			loader.write(data)
		
		if exc_type:
			loader.close()
			handle.close(lambda *args: None)
			self.ui.get_widget("plot").set_from_pixbuf(loader.get_pixbuf())
			progress.set_text(url)
		else:
			progress.set_text(_("Downloading Chart"))
			handle.read(GNOMEVFS_CHUNK_SIZE, self.on_chart_read, udata)
		
	def on_autorefresh_toggled(self, autorefresh):
		if self.autorefresh_id != 0:
			gobject.source_remove(self.autorefresh_id)
			self.autorefresh_id = 0
			
		if autorefresh.get_active():
			self.autorefresh_id = gobject.timeout_add(AUTOREFRESH_TIMEOUT, self.on_refresh_chart, True)

def show_chart(tickers):
	ui = gtk.glade.XML(os.path.join(invest.SHARED_DATA_DIR, "financialchart.glade"))
	chart = FinancialChart(ui)
	ui.get_widget("s").set_text(' '.join(tickers))
	chart.on_refresh_chart()
	return ui.get_widget("window")

