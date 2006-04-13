#!/usr/bin/env python
#

import gobject
import gtk, gnomeapplet

import getopt, sys
from os.path import *

# Allow to use uninstalled
def _check(path):
	return exists(path) and isdir(path) and isfile(path+"/ChangeLog")

name = join(dirname(__file__), '..')
if _check(name):
	print 'Running uninstalled invest, modifying PYTHONPATH'
	sys.path.insert(0, abspath(name))
else:
	sys.path.insert(0, abspath("@PYTHONDIR@"))
	print "Running installed invest, using [@PYTHONDIR@:$PYTHONPATH]"

# Now the path is set, import our applet
import invest, invest.applet, invest.defs

import gettext, locale
gettext.bindtextdomain('invest-applet', abspath(join(invest.defs.DATA_DIR, "locale")))
gettext.textdomain('invest-applet')

locale.bindtextdomain('invest-applet', abspath(join(invest.defs.DATA_DIR, "locale")))
locale.textdomain('invest-applet')

from gettext import gettext as _

def applet_factory(applet, iid):
	print 'Starting invest instance:', applet, iid
	invest.applet.InvestApplet(applet)
	return True

# Return a standalone window that holds the applet
def build_window():
	app = gtk.Window(gtk.WINDOW_TOPLEVEL)
	app.set_title(_("Invest Applet"))
	app.connect("destroy", gtk.main_quit)
	app.set_property('resizable', False)
	
	applet = gnomeapplet.Applet()
	applet_factory(applet, None)
	applet.reparent(app)
		
	app.show_all()
	
	return app
		
		
def usage():
	print """=== Invest applet: Usage
$ invest-applet [OPTIONS]

OPTIONS:
	-h, --help			Print this help notice.
	-d, --debug			Enable debug output (default=off).
	-w, --window		Launch the applet in a standalone window for test purposes (default=no).
	"""
	sys.exit()
	
if __name__ == "__main__":	
	standalone = False
	
	try:
		opts, args = getopt.getopt(sys.argv[1:], "hdw", ["help", "debug", "window"])
	except getopt.GetoptError:
		# Unknown args were passed, we fallback to bahave as if
		# no options were passed
		opts = []
		args = sys.argv[1:]
	
	for o, a in opts:
		if o in ("-h", "--help"):
			usage()
		elif o in ("-d", "--debug"):
			print "No problems so far."
		elif o in ("-w", "--window"):
			standalone = True
			
	if standalone:
		import gnome
		gnome.init(invest.defs.PACKAGE, invest.defs.VERSION)
		build_window()
		gtk.main()
	else:
		gnomeapplet.bonobo_factory(
			"OAFIID:Invest_Applet_Factory",
			gnomeapplet.Applet.__gtype__,
			invest.defs.PACKAGE,
			invest.defs.VERSION,
			applet_factory)
