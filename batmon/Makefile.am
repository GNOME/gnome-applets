pixmapdir = $(datadir)/pixmaps

INCLUDES =	 			\
	-I../../..	 		\
	-I$(srcdir)/../../..	 	\
	-I$(includedir)

lib_LTLIBRARIES = libpanel_batmon.la

libpanel_batmon_la_SOURCES = 	\
	batmon.c

EXTRA_DIST  = batmon-ac.xpm batmon-bat.xpm

pixmap_DATA = batmon-ac.xpm batmon-bat.xpm
