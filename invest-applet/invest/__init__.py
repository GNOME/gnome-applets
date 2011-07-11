import os, sys, traceback
from os.path import join, exists, isdir, isfile, dirname, abspath, expanduser
from types import ListType
import datetime
from gi.repository import GObject, Gtk, Gdk, GConf
import cPickle
import networkmanager

# Autotools set the actual data_dir in defs.py
from defs import *

DEBUGGING = False

# central debugging and error method
def debug(msg):
	if DEBUGGING:
		print "%s: %s" % (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"), msg)

def error(msg):
	print "%s: ERROR: %s" % (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"), msg)


def exceptionhandler(t, value, tb):
	error("%s occurred: %s" % (t.__name__, value))
	for elem in traceback.extract_tb(tb):
		error("\t%s:%d in %s: %s" % (elem[0], elem[1], elem[2], elem[3]))

# redirect uncaught exceptions to this exception handler
debug("Installing default exception handler")
sys.excepthook = exceptionhandler



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
GCONF_CLIENT = GConf.Client.get_default()

# GConf directory for invest in window mode and shared settings
GCONF_DIR = "/apps/invest"

# GConf key for list of enabled handlers, when uninstalled, use a debug key to not conflict
# with development version
#GCONF_ENABLED_HANDLERS = GCONF_DIR + "/enabled_handlers"

# Preload gconf directories
#GCONF_CLIENT.add_dir(GCONF_DIR, gconf.CLIENT_PRELOAD_RECURSIVE)

# tests whether the given stocks are in the old labelless format
def labelless_stock_format(stocks):
	if len(stocks) == 0:
		return False

	# take the first element of the dict and check if its value is a list
	if type(stocks[stocks.keys()[0]]) is ListType:
		return True

	# there is no list, so it is already the new stock file format
	return False

# converts the given stocks from the labelless format into the one with labels
def update_to_labeled_stock_format(stocks):
	new = {}

	for k, l in stocks.items():
		d = {'label':"", 'purchases':l}
		new[k] = d

	return new

# tests whether the given stocks are in the format without exchange information
def exchangeless_stock_format(stocks):
	if len(stocks) == 0:
		return False

	# take the first element of the dict and check if its value is a list
	for symbol, data in stocks.items():
		purchases = stocks[symbol]["purchases"]
		if len(purchases) > 0:
			purchase = purchases[0]
			if not purchase.has_key("exchange"):
				return True

	return False

# converts the given stocks into format with exchange information
def update_to_exchange_stock_format(stocks):
	for symbol, data in stocks.items():
		purchases = data["purchases"]
		for purchase in purchases:
			purchase["exchange"] = 0

	return stocks

# converts the given stocks from the dict format into a list
def update_to_list_stock_format(stocks):
	new = []

	for ticker, stock in stocks.items():
		stock['ticker'] = ticker
		new.append(stock)

	return new

STOCKS_FILE = join(USER_INVEST_DIR, "stocks.pickle")

try:
	STOCKS = cPickle.load(file(STOCKS_FILE))

	# if the stocks file contains a list, the subsequent tests are obsolete
	if type(STOCKS) != list:
		# if the stocks file is in the stocks format without labels,
		# then we need to convert it into the new labeled format
		if labelless_stock_format(STOCKS):
			STOCKS = update_to_labeled_stock_format(STOCKS)

		# if the stocks file does not contain exchange rates, add them
		if exchangeless_stock_format(STOCKS):
			STOCKS = update_to_exchange_stock_format(STOCKS)

		# here, stocks is a most up-to-date dict, but we need it to be a list
		STOCKS = update_to_list_stock_format(STOCKS)

except Exception, msg:
	error("Could not load the stocks from %s: %s" % (STOCKS_FILE, msg) )
	STOCKS = []

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

CONFIG_FILE = join(USER_INVEST_DIR, "config.pickle")
try:
	CONFIG = cPickle.load(file(CONFIG_FILE))
except Exception, msg:
	error("Could not load the configuration from %s: %s" % (CONFIG_FILE, msg) )
	CONFIG = {}       # default configuration

CURRENCIES_FILE = join(USER_INVEST_DIR, "currencies.csv")
QUOTES_FILE = join(USER_INVEST_DIR, "quotes.csv")
INDEX_QUOTES_FILE_TEMPLATE = "quotes.#.csv"


# set default proxy config
PROXY = None

# borrowed from Ross Burton
# http://burtonini.com/blog/computers/postr
# extended by exception handling and retry scheduling
def get_gnome_proxy(client):
	sleep = 10	# sleep between attempts for 10 seconds
	attempts = 3	# try to get configuration from gconf at most three times
	get_gnome_proxy_retry(client, attempts, sleep)

def get_gnome_proxy_retry(client, attempts, sleep):
	# decrease attempts counter
	attempts -= 1

	# sanity check if we still need to look for proxy configuration
	global PROXY
	if PROXY != None:
		return

	# try to get config from gconfd
	try:
		if client.get_bool("/system/http_proxy/use_http_proxy"):
			host = client.get_string("/system/http_proxy/host")
			port = client.get_int("/system/http_proxy/port")
			if host is None or host == "" or port == 0:
				# gnome proxy is not valid, stop here
				return

			if client.get_bool("/system/http_proxy/use_authentication"):
				user = client.get_string("/system/http_proxy/authentication_user")
				password = client.get_string("/system/http_proxy/authentication_password")
				if user and user != "":
					url = "http://%s:%s@%s:%d" % (user, password, host, port)
				else:
					url = "http://%s:%d" % (host, port)
			else:
				url = "http://%s:%d" % (host, port)

			# proxy config found, memorize
			PROXY = {'http': url}

	except Exception, msg:
		error("Failed to get proxy configuration from GConfd:\n%s" % msg)
		# we did not succeed, schedule retry
		if attempts > 0:
			error("Retrying to contact GConfd in %d seconds" % sleep)
			GObject.timeout_add(sleep * 1000, get_gnome_proxy_retry, client, attempts, sleep)

# use gconf to get proxy config
debug("Detecting proxy settings")
client = GConf.Client.get_default()
get_gnome_proxy(client)


# connect to Network Manager to identify current network connectivity
debug("Connecting to the NetworkManager")
nm = networkmanager.NetworkManager()
