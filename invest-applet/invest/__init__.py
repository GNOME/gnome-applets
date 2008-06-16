import os, sys
from os.path import join, exists, isdir, isfile, dirname, abspath, expanduser

import gtk, gtk.gdk, gconf
import cPickle

# Autotools set the actual data_dir in defs.py
from defs import *

DEBUGGING = False

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
	BUILDER_DATA_DIR = SHARED_DATA_DIR
	ART_DATA_DIR = join(SHARED_DATA_DIR, 'art')
else:
	SHARED_DATA_DIR = join(DATA_DIR, "gnome-applets", "invest-applet")
	BUILDER_DATA_DIR = BUILDERDIR
	ART_DATA_DIR = SHARED_DATA_DIR
if DEBUGGING:
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

#Gconf client
GCONF_CLIENT = gconf.client_get_default()

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

QUOTES_URL="http://finance.yahoo.com/d/quotes.csv?s=%(s)s&f=sl1d1t1c1ohgv&e=.csv"

# Sample (25/4/2008): UCG.MI,"4,86",09:37:00,2008/04/25,"0,07","4,82","4,87","4,82",11192336
QUOTES_CSV_FIELDS=["ticker", ("trade", float), "time", "date", ("variation", float), ("open", float)]

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
