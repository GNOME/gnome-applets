pixmapdir = $(datadir)/panel/

INCLUDES = 			\
	-I../../.. 		\
	-I$(srcdir)/../../.. 	\
	-I$(includedir)		\
	$(X_CFLAGS)

lib_LTLIBRARIES = libpanel_netwatch.la

libpanel_netwatch_la_SOURCES = 	\
	netwatch.c

# See ../panelrc.in for where these xpms are reference from

pixmap_DATA = \
	netwatch-modem-up.xpm \
	netwatch-modem-down.xpm \
	netwatch-net-up.xpm \
	netwatch-net-down.xpm

EXTRA_DIST = $(pixmap_DATA)

