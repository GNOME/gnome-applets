# Makefile.am
#
# Author: Jörgen Pehrson <jp@spektr.eu.org>
# Created: Thu Jun 15 23:08:41 CEST 2000
#

gladedir = $(datadir)/battstat_applet
glade_DATA = battstat_applet.glade


INCLUDES = \
	-I$(top_srcdir)/intl \
	-I$(top_srcdir) \
	-I$(includedir) \
	$(GNOME_INCLUDEDIR) \
	-DDATA_DIR=\"$(datadir)\" \
	-DGLADE_DIR=\"$(gladedir)\" \
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = battstat_applet

battstat_applet_CFLAGS = -DAPPLET 

battstat_applet_SOURCES = battstat.h pixmaps.h properties.c battstat_applet.c 

battstat_applet_LDADD = 			\
			$(GNOME_LIBDIR) 	\
			$(GNOME_APPLETS_LIBS)   \
	 		${GNOME_DOCKLETS_LIBS}	\
			-lglade			\
			-lglade-gnome

EXTRADIST = $(glade_DATA)


