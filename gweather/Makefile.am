## Process this file with automake to produce Makefile.in

SUBDIRS = docs

INCLUDES =					\
	-I$(srcdir)				\
	-I$(top_srcdir)				\
	-DGWEATHER_MENU_UI_DIR=\""$(uidir)"\"	\
	$(DBUS_CFLAGS)				\
	$(GNOME_APPLETS_CFLAGS)		\
	$(DBUS_CFLAGS)				\
	$(LIBNOTIFY_CFLAGS)			\
	$(LIBGWEATHER_CFLAGS)			\
	$(NETWORKMANAGER_CFLAGS)		\
	$(WARN_CFLAGS)

libexec_PROGRAMS = gweather-applet-2

gweather_applet_2_SOURCES = \
	gweather.h \
	main.c \
	gweather-about.c gweather-about.h \
	gweather-pref.c gweather-pref.h \
	gweather-dialog.c gweather-dialog.h \
	gweather-applet.c gweather-applet.h

gweather_applet_2_LDADD = 	\
	$(DBUS_LIBS)		\
	$(LIBNOTIFY_LIBS) 	\
	$(GNOME_APPLETS_LIBS)	\
	$(DBUS_LIBS)		\
	$(GNOME_LIBS2_LIBS)	\
	$(LIBGWEATHER_LIBS) 	\
	$(NETWORKMANAGER_LIBS)

uidir = $(pkgdatadir)/ui
ui_DATA = gweather-applet-menu.xml

appletdir       = $(datadir)/gnome-panel/4.0/applets
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

%.gschema.xml.in: %.gschema.xml.in.in
	$(AM_V_GEN)sed -e "s|\@GETTEXT_PACKAGE\@|$(GETTEXT_PACKAGE)|" $< > $@

@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@

gsettings_SCHEMAS = $(builddir)/org.gnome.applets.GWeatherApplet.gschema.xml

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA)

EXTRA_DIST = \
	org.gnome.applets.GWeatherApplet.panel-applet.in.in	\
	org.gnome.applets.GWeatherApplet.gschema.xml		\
	$(service_in_files)					\
	$(ui_DATA)

-include $(top_srcdir)/git.mk
