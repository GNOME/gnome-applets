# Makefile.am
#
# Author: Jörgen Pehrson <jp@spektr.eu.org>
# Created: Thu Jun 15 23:08:41 CEST 2000
#

gladedir = $(datadir)/battstat_applet
glade_DATA = battstat_applet.glade

if HAVE_LIBAPM
APMLIB =
APMDIR =
else
APMLIB = ../apmlib/libapm.a
APMDIR = -I$(top_srcdir)/apmlib
endif

INCLUDES = \
	-I$(top_srcdir)/intl \
	-I$(top_srcdir) \
	-I$(includedir) \
	$(gladeincludedir) \
	$(BATTSTAT_CFLAGS) \
	-DDATA_DIR=\"$(datadir)\" \
	-DGLADE_DIR=\"$(gladedir)\" \
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
#	-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
#	-DGDK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED 
	$(APMDIR) $(NULL)

bin_PROGRAMS = battstat_applet

battstat_applet_CFLAGS = -DAPPLET 

battstat_applet_SOURCES = battstat.h pixmaps.h properties.c battstat_applet.c acpi-linux.c acpi-linux.h

battstat_applet_LDADD = 			\
	 		${BATTSTAT_LIBS}	\
			-lglade			\
			-lglade-gnome           \
			$(APMLIB)

EXTRA_DIST = $(glade_DATA)


