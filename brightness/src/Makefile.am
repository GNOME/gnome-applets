NULL =

uidir   = $(pkgdatadir)/ui

libexec_PROGRAMS=gnome-brightness-applet

gnome_brightness_applet_CFLAGS = \
	-I. \
	-I$(srcdir) \
	-DBRIGHTNESS_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

gnome_brightness_applet_SOURCES = \
	$(BUILT_SOURCES) \
	brightness-applet.c \
	$(NULL)

gnome_brightness_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

gnome_brightness_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
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

-include $(top_srcdir)/git.mk
