# -*- coding: utf-8 -*-
from gi.repository import Gtk, Gdk

def show_help():
	Gtk.show_uri(None, "help:invest-applet", Gdk.CURRENT_TIME)

def show_help_section(id):
	Gtk.show_uri(None, "help:invest-applet/%s" % id, Gdk.CURRENT_TIME)
