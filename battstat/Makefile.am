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
	$(APMDIR) $(NULL)

#	-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
#	-DGDK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED 

bin_PROGRAMS = battstat-applet-2

battstat_applet_2_CFLAGS = -DAPPLET 

battstat_applet_2_SOURCES = battstat.h pixmaps.h properties.c battstat_applet.c acpi-linux.c acpi-linux.h

battstat_applet_2_LDADD = 			\
	 		${BATTSTAT_LIBS}	\
			-lglade			\
			-lglade-gnome           \
			$(APMLIB)

EXTRA_DIST = 				\
	$(glade_DATA)			\
	GNOME_BattstatApplet.server.in  \
	battstat.schemas

schemadir   = $(sysconfdir)/gconf/schemas
schema_DATA = battstat.schemas

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_BattstatApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

