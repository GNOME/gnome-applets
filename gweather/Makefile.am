## Process this file with automake to produce Makefile.in

# SUBDIRS = pixmaps docs
SUBDIRS = pixmaps

INCLUDES = \
	-I$(srcdir)	-I$(srcdir)/pixmaps	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DGTK_ENABLE_BROKEN	\
	$(GNOME_APPLETS_CFLAGS)	\
	$(GNOME_VFS_APPLETS_CFLAGS)

bin_PROGRAMS = gweather-applet-2

gweather_applet_2_SOURCES = \
	gweather.h \
	main.c \
	gweather-about.c gweather-about.h \
	gweather-pref.c gweather-pref.h \
	gweather-dialog.c gweather-dialog.h \
	gweather-applet.c gweather-applet.h \
	weather.c weather.h

gweather_applet_2_LDADD = \
	$(GNOME_APPLETS_LIBS)	\
	$(GNOME_VFS_APPLETS_LIBS)

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= gweather.schemas

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GWeatherApplet.xml

install-data-local:
	if test -z "$(DESTDIR)" ; then \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;\
	fi
	
serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GWeatherApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

gweatherlocationsdir = $(datadir)/gweather
gweatherlocations_DATA = Locations

EXTRA_DIST = \
	GNOME_GWeatherApplet_Factory.server.in \
	gweather.schemas \
	$(ui_DATA)	\
	Locations

@INTLTOOL_SERVER_RULE@
