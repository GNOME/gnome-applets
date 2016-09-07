NULL =

uidir = $(pkgdatadir)/ui

libexec_PROGRAMS = gweather-applet-2

gweather_applet_2_CFLAGS = \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-DGWEATHER_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	$(DBUS_CFLAGS) \
	$(GNOME_APPLETS_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(LIBGWEATHER_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

gweather_applet_2_SOURCES = \
	gweather.h \
	gweather-about.c \
	gweather-about.h \
	gweather-pref.c \
	gweather-pref.h \
	gweather-xml.c \
	gweather-xml.h \
	gweather-dialog.c \
	gweather-dialog.h \
	gweather-applet.c \
	gweather-applet.h \
	main.c \
	$(NULL)

gweather_applet_2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

gweather_applet_2_LDADD = \
	$(DBUS_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(GNOME_APPLETS_LIBS) \
	$(DBUS_LIBS) \
	$(GNOME_LIBS2_LIBS) \
	$(LIBGWEATHER_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
