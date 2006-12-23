## Process this file with automake to produce Makefile.in

SUBDIRS = docs

INCLUDES =				\
	-I$(srcdir)			\
	-I$(top_srcdir)			\
	-DGTK_ENABLE_BROKEN		\
	$(GNOME_APPLETS_CFLAGS)		\
	$(GNOME_VFS_APPLETS_CFLAGS)	\
	$(LIBNOTIFY_CFLAGS)		\
	$(XML2_CFLAGS)			\
	-DGWEATHER_XML_LOCATION=\""$(pkgdatadir)/gweather/"\"

libexec_PROGRAMS = gweather-applet-2

gweather_applet_2_SOURCES = \
	gweather.h \
	main.c \
	gweather-about.c gweather-about.h \
	gweather-pref.c gweather-pref.h \
	gweather-dialog.c gweather-dialog.h \
	gweather-applet.c gweather-applet.h \
	gweather-xml.c gweather-xml.h

gweather_applet_2_LDADD = \
	$(LIBNOTIFY_LIBS) \
	$(GNOME_APPLETS_LIBS)	\
	$(GNOME_VFS_APPLETS_LIBS) \
	$(XML2_LIBS) \
	../libgweather/libgweather.la

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GWeatherApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GWeatherApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

## this will eventually be moved to the gnome-location-db
gweatherlocationsdir = $(pkgdatadir)/gweather
gweatherlocations_in_files = Locations.xml.in
gweatherlocations_DATA = $(gweatherlocations_in_files:.xml.in=.xml) locations.dtd
%.xml:       %.xml.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po-locations/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po-locations/.intltool-merge-cache $(top_srcdir)/po-locations $< $@

EXTRA_DIST = \
	GNOME_GWeatherApplet_Factory.server.in.in \
	$(ui_DATA)	 \
	Locations.xml \
	locations.dtd

@INTLTOOL_SERVER_RULE@
