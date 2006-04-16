import os, sys
from os.path import join, exists, isdir, isfile, dirname, abspath, expanduser

import gtk, gtk.gdk, gconf
import cPickle

# Autotools set the actual data_dir in defs.py
from defs import *

# Allow to use uninstalled invest ---------------------------------------------
UNINSTALLED_INVEST = False
def _check(path):
	return exists(path) and isdir(path) and isfile(path+"/ChangeLog")
	
name = join(dirname(__file__), '..')
if _check(name):
	UNINSTALLED_INVEST = True
	
# Sets SHARED_DATA_DIR to local copy, or the system location
# Shared data dir is most the time /usr/share/invest-applet
if UNINSTALLED_INVEST:
	SHARED_DATA_DIR = abspath(join(dirname(__file__), '..', 'data'))
	GLADE_DATA_DIR = SHARED_DATA_DIR
else:
	SHARED_DATA_DIR = join(DATA_DIR, "gnome-applets", "invest-applet")
	GLADE_DATA_DIR = join(SHARED_DATA_DIR, "glade")
print "Data Dir: %s" % SHARED_DATA_DIR

USER_INVEST_DIR = expanduser("~/.gnome2/invest-applet")
if not exists(USER_INVEST_DIR):
	try:
		os.makedirs(USER_INVEST_DIR, 0744)
	except Exception , msg:
		print 'Error:could not create user dir (%s): %s' % (USER_INVEST_DIR, msg)
# ------------------------------------------------------------------------------

# Set the cwd to the home directory so spawned processes behave correctly
# when presenting save/open dialogs
os.chdir(expanduser("~"))

# Path to images, icons
ART_DATA_DIR = join(SHARED_DATA_DIR, "art")

#Gconf client
#GCONF_CLIENT = gconf.client_get_default()

# GConf directory for invest in window mode and shared settings
GCONF_DIR = "/apps/invest"

# GConf key for list of enabled handlers, when uninstalled, use a debug key to not conflict
# with development version
#GCONF_ENABLED_HANDLERS = GCONF_DIR + "/enabled_handlers"
	
# Preload gconf directories
#GCONF_CLIENT.add_dir(GCONF_DIR, gconf.CLIENT_PRELOAD_RECURSIVE)

STOCKS_FILE = join(USER_INVEST_DIR, "stocks.pickle")

GNOMEVFS_CHUNK_SIZE = 512*1024 # 512 KBytes
AUTOREFRESH_TIMEOUT = 20*60*1000 # 15 minutes
TICKER_TIMEOUT = 10000#3*60*1000#

CHART_BASE_URL = "http://ichart.finance.yahoo.com/z?s=%(s)s&t=%(t)s&q=%(q)s&l=%(l)s&z=%(z)s&p=%(p)s&a=%(a)s%(opt)s"
QUOTES_URL="http://finance.yahoo.com/d/quotes.csv?s=%(s)s&f=sl1d1t1c1ohgv&e=.csv"

# Sample: "APPL",76.05,"1/9/2006","4:00pm",0.00,N/A,N/A,N/A,500
QUOTES_CSV_FIELDS=["ticker", ("trade", float), "date", "time"]

try:
	STOCKS = cPickle.load(file(STOCKS_FILE))
except Exception, msg:
	STOCKS = {}
	
#STOCKS = {
#	"AAPL": {
#		"amount": 12,
#		"bought": 74.94,
#		"comission": 31,
#	},
#	"INTC": {
#		"amount": 30,
#		"bought": 25.85,
#		"comission": 31,
#	},
#	"GOOG": {
#		"amount": 1,
#		"bought": 441.4,
#		"comission": 31,
#	},
#}
