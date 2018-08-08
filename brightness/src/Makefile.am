NULL =

uidir   = $(pkgdatadir)/ui

brightness_applet_libdir = $(pkglibdir)
brightness_applet_lib_LTLIBRARIES = libbrightness-applet.la

libbrightness_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DBRIGHTNESS_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.brightness\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libbrightness_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libbrightness_applet_la_SOURCES = \
	$(BUILT_SOURCES) \
	brightness-applet.c \
	$(NULL)

libbrightness_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libbrightness_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(NULL)

BUILT_SOURCES = \
	dbus-brightness.c \
	dbus-brightness.h \
	$(NULL)

dbus-brightness.h:
dbus-brightness.c: org.gnome.SettingsDaemon.Power.Screen.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix=org.gnome. \
		--generate-c-code dbus-brightness \
		--c-namespace DBus \
		$(srcdir)/org.gnome.SettingsDaemon.Power.Screen.xml

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(NULL)

EXTRA_DIST = \
	org.gnome.SettingsDaemon.Power.Screen.xml \
	$(NULL)

-include $(top_srcdir)/git.mk
