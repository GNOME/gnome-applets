## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps docs

INCLUDES = \
	-I$(srcdir)	-I$(srcdir)/pixmaps	\
	-I$(top_srcdir)/screen-exec		\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DGTK_ENABLE_BROKEN	\
	$(GNOME_APPLETS_CFLAGS)	\
	$(GNOME_VFS_APPLETS_CFLAGS)

libexec_PROGRAMS = gweather-applet-2

gweather_applet_2_SOURCES = \
	gweather.h \
	main.c \
	gweather-about.c gweather-about.h \
	gweather-pref.c gweather-pref.h \
	gweather-dialog.c gweather-dialog.h \
	gweather-applet.c gweather-applet.h \
	weather.c weather.h

gweather_applet_2_LDADD = \
	$(top_builddir)/screen-exec/libscreen-exec.la \
	$(GNOME_APPLETS_LIBS)	\
	$(GNOME_VFS_APPLETS_LIBS)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = gweather.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GWeatherApplet.xml

install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA) ;
	
serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GWeatherApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@
	
gweatherlocationsdir = $(datadir)/gweather
gweatherlocations_DATA = Locations


EXTRA_DIST = \
	GNOME_GWeatherApplet_Factory.server.in.in \
	$(schemas_in_files) \
	$(ui_DATA)	 \
	Locations

@INTLTOOL_SERVER_RULE@
