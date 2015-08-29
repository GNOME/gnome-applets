SUBDIRS = icons

AM_CPPFLAGS =					\
	-I.					\
	-I$(srcdir)				\
	-DBRIGHTNESS_MENU_UI_DIR=\""$(uidir)"\"	\
	$(GNOME_APPLETS_CFLAGS)			\
	$(WARN_CFLAGS)				\
	$(NULL)

libexec_PROGRAMS=gnome-brightness-applet

gnome_brightness_applet_SOURCES =				\
	dbus-brightness.h					\
	dbus-brightness.c					\
	brightness-applet.c

gnome_brightness_applet_LDADD =					\
	$(GNOME_APPLETS_LIBS)

dbus-brightness.h:
dbus-brightness.c: org.gnome.SettingsDaemon.Power.Screen.xml Makefile.am
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix=org.gnome. \
		--generate-c-code dbus-brightness \
		--c-namespace DBus \
		org.gnome.SettingsDaemon.Power.Screen.xml

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.BrightnessApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.BrightnessAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.BrightnessAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

uidir   = $(pkgdatadir)/ui
ui_DATA = brightness-applet-menu.xml

BUILT_SOURCES = \
	dbus-brightness.c \
	dbus-brightness.h \
	$(NULL)

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(BUILT_SOURCES) $(service_DATA)

EXTRA_DIST = \
	org.gnome.BrightnessApplet.panel-applet.in.in \
	$(ui_DATA) \
	$(service_in_files)

-include $(top_srcdir)/git.mk
