
gladedir = $(datadir)/battstat_applet
glade_DATA = battstat_applet.glade

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = battstat.png

if HAVE_LIBAPM
APMLIB = -lapm
APMDIR =
APMINC = 
else
APMLIB = apmlib/libapm.a
APMDIR = apmlib
APMINC = -Iapmlib/
endif

SUBDIRS = sounds $(APMDIR)

DIST_SUBDIRS = sounds apmlib

INCLUDES = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBGLADE_CFLAGS) \
	$(APMINC) \
	-DDATA_DIR=\"$(datadir)\" \
	-DGLADE_DIR=\"$(gladedir)\" \
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 


bin_PROGRAMS = battstat-applet-2


battstat_applet_2_SOURCES = \
	battstat.h \
	pixmaps.h \
	properties.c \
	battstat_applet.c \
	acpi-linux.c \
	acpi-linux.h


battstat_applet_2_LDADD = \
		     $(GNOME_APPLETS_LIBS) \
		     $(LIBGLADE_LIBS) \
		     $(APMLIB)

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = battstat.schemas

install-data-local:
	if test -z "$(DESTDIR)" ; then \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;\
	fi

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_BattstatApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_BattstatApplet.xml

EXTRA_DIST = 				\
	$(glade_DATA)			\
	GNOME_BattstatApplet.server.in  \
	$(ui_DATA)			\
	battstat.schemas		\
	battstat.png

@INTLTOOL_SERVER_RULE@

