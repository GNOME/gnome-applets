## Process this file with automake to produce Makefile.in

SUBDIRS = docs

INCLUDES =				\
	-I$(srcdir)			\
	-I$(top_srcdir)			\
	$(GNOME_APPLETS_CFLAGS)		\
	$(GNOME_LIBS2_CFLAGS)		\
	$(LIBNOTIFY_CFLAGS)		\
	$(LIBGWEATHER_CFLAGS)		\
	$(NETWORKMANAGER_CFLAGS)

libexec_PROGRAMS = gweather-applet-2

gweather_applet_2_SOURCES = \
	gweather.h \
	main.c \
	gweather-about.c gweather-about.h \
	gweather-pref.c gweather-pref.h \
	gweather-dialog.c gweather-dialog.h \
	gweather-applet.c gweather-applet.h

gweather_applet_2_LDADD = 	\
	$(LIBNOTIFY_LIBS) 	\
	$(GNOME_APPLETS_LIBS)	\
	$(GNOME_LIBS2_LIBS)	\
	$(LIBGWEATHER_LIBS) 	\
	$(NETWORKMANAGER_LIBS)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GWeatherApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GWeatherApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

EXTRA_DIST = \
	GNOME_GWeatherApplet_Factory.server.in.in \
	$(ui_DATA)

@INTLTOOL_SERVER_RULE@

-include $(top_srcdir)/git.mk
