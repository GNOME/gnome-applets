## Process this file with automake to produce Makefile.in

SUBDIRS = docs

INCLUDES =					\
	-I$(srcdir)				\
	-I$(top_srcdir)				\
	-DGWEATHER_MENU_UI_DIR=\""$(uidir)"\"	\
	$(GNOME_APPLETS3_CFLAGS)		\
	$(LIBNOTIFY_CFLAGS)			\
	$(LIBGWEATHER_CFLAGS)			\
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
	$(GNOME_APPLETS3_LIBS)	\
	$(GNOME_LIBS2_LIBS)	\
	$(LIBGWEATHER_LIBS) 	\
	$(NETWORKMANAGER_LIBS)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = gweather-applet-menu.xml

appletdir       = $(datadir)/gnome-panel/applets
applet_in_files = org.gnome.applets.GWeatherApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.GWeatherAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.GWeatherAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA)

EXTRA_DIST = \
	org.gnome.applets.GWeatherApplet.panel-applet.in.in	\
	$(service_in_files)					\
	$(ui_DATA)

-include $(top_srcdir)/git.mk
