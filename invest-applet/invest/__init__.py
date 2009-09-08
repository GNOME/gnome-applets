import os, sys
from os.path import join, exists, isdir, isfile, dirname, abspath, expanduser
from types import ListType
import datetime

import gtk, gtk.gdk, gconf, gobject
import cPickle

# Autotools set the actual data_dir in defs.py
from defs import *

DEBUGGING = False

# central debugging and error method
def debug(msg):
	if DEBUGGING:
		print "%s: %s" % (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"), msg)

def error(msg):
	print "%s: ERROR: %s" % (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"), msg)


# Allow to use uninstalled invest ---------------------------------------------
UNINSTALLED_INVEST = False
def _check(path):
	return exists(path) and isdir(path) and isfile(path+"/Makefile.am")

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

USER_INVEST_DIR = expanduser("~/.gnome2/invest-applet")
if not exists(USER_INVEST_DIR):
	try:
		os.makedirs(USER_INVEST_DIR, 0744)
	except Exception , msg:
		error('Could not create user dir (%s): %s' % (USER_INVEST_DIR, msg))
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

# tests whether the given stocks are in the old format
def old_stock_format(stocks):
	if len(stocks) == 0:
		return False

	# take the first element of the dict and check if its value is a list
	if type(stocks[stocks.keys()[0]]) is ListType:
		return True

	# there is no list, so it is already the new stock file format
	return False

# converts the given stocks from the old format into the new one
def update_stock_format(stocks):
	new = {}

	for k, l in stocks.items():
		d = {'label':"", 'purchases':l}
		new[k] = d

	return new

STOCKS_FILE = join(USER_INVEST_DIR, "stocks.pickle")

try:
	STOCKS = cPickle.load(file(STOCKS_FILE))

	# if the stocks file is in the old stocks format,
	# then we need to convert it into the new format
	if old_stock_format(STOCKS):
		STOCKS = update_stock_format(STOCKS);
except Exception, msg:
	error("Could not load the stocks from %s: %s" % (STOCKS_FILE, msg) )
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

client = gconf.client_get_default()

# borrowed from Ross Burton
# http://burtonini.com/blog/computers/postr
def get_gnome_proxy(client):
	if client.get_bool("/system/http_proxy/use_http_proxy"):
		host = client.get_string("/system/http_proxy/host")
		port = client.get_int("/system/http_proxy/port")
		if host is None or host == "" or port == 0:
			# gnome proxy is not valid, use enviroment if available
			return None

		if client.get_bool("/system/http_proxy/use_authentication"):
			user = client.get_string("/system/http_proxy/authentication_user")
			password = client.get_string("/system/http_proxy/authentication_password")
			if user and user != "":
				url = "http://%s:%s@%s:%d" % (user, password, host, port)
			else:
				url = "http://%s:%d" % (host, port)
		else:
			url = "http://%s:%d" % (host, port)

		return {'http': url}
	else:
		# gnome proxy is not set, use enviroment if available
		return None

PROXY = get_gnome_proxy(client)
