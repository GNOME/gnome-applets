#!/usr/bin/env python

from gi.repository import GObject, Gtk, PanelApplet
import getopt, sys
from os.path import *

# Allow to use uninstalled
def _check(path):
	return exists(path) and isdir(path) and isfile(path+"/Makefile.am")

name = join(dirname(__file__), '..')
if _check(name):
	print 'Running uninstalled invest, modifying PYTHONPATH'
	sys.path.insert(0, abspath(name))
else:
	sys.path.insert(0, abspath("@PYTHONDIR@"))

# Now the path is set, import our applet
import invest, invest.applet, invest.defs, invest.help, invest.networkmanager

# Prepare i18n
import gettext, locale
gettext.bindtextdomain(invest.defs.GETTEXT_PACKAGE, invest.defs.GNOMELOCALEDIR)
gettext.textdomain(invest.defs.GETTEXT_PACKAGE)
locale.bindtextdomain(invest.defs.GETTEXT_PACKAGE, invest.defs.GNOMELOCALEDIR)
locale.textdomain(invest.defs.GETTEXT_PACKAGE)

from gettext import gettext as _

def applet_factory(applet, iid, data):
	invest.debug('Starting invest instance: %s %s %s'% ( applet, iid, data ))
	invest.applet.InvestApplet(applet)
	return True


def usage():
	print """=== Invest applet: Usage
$ invest-applet [OPTIONS]

OPTIONS:
	-h, --help			Print this help notice.
	-d, --debug			Enable debug output (default=off).

For test purposes, you can run the applet as a standalone window by running (e.g.)
  /usr/libexec/invest-applet -d
and executing the panel-applet-test application:
  env panel-test-applets
Click 'execute'. Make sure no invest applet is currently running in the panel.
If invest is not installed to the system, instead execute
  env GNOME_PANEL_APPLETS_DIR=/PATH/TO/INVEST_SOURCES/data panel-test-applets
	"""
	sys.exit()

if __name__ == "__main__":
	try:
		opts, args = getopt.getopt(sys.argv[1:], "hdw", ["help", "debug", "window"])
	except getopt.GetoptError:
		# Unknown args were passed, we fallback to behave as if
		# no options were passed
		opts = []
		args = sys.argv[1:]

	for o, a in opts:
		if o in ("-h", "--help"):
			usage()
		elif o in ("-d", "--debug"):
			invest.DEBUGGING = True
			invest.debug("Debugging enabled")
			# these messages cannot be turned by invest.DEBUGGING at their originating location,
			# because that variable was set here to be True
			invest.debug("Data Dir: %s" % invest.SHARED_DATA_DIR)
			invest.debug("Detected PROXY: %s" % invest.PROXY)
			invest.debug("Found NetworkManager spec %s (%s)" % (invest.networkmanager.spec, invest.networkmanager.version))

	invest.debug("Starting applet factory, waiting for gnome-panel to connect ...")
	PanelApplet.Applet.factory_main(
		"InvestAppletFactory",	# id
		PanelApplet.Applet.__gtype__,	# gtype
		applet_factory,			# factory callback
		None)				# factory data pointer
