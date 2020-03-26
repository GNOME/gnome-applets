NULL =

uidir = $(pkgdatadir)/ui
ui_DATA = gweather-applet-menu.xml

noinst_LTLIBRARIES = libgweather-applet.la

libgweather_applet_la_CPPFLAGS = \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-DGWEATHER_MENU_UI_DIR=\""$(uidir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.gweather\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libgweather_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBGWEATHER_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(XML2_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libgweather_applet_la_SOURCES = \
	gweather-pref.c \
	gweather-pref.h \
	gweather-xml.c \
	gweather-xml.h \
	gweather-dialog.c \
	gweather-dialog.h \
	gweather-applet.c \
	gweather-applet.h \
	$(NULL)

libgweather_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libgweather_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(LIBGWEATHER_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(XML2_LIBS) \
	$(NULL)

EXTRA_DIST = \
	$(ui_DATA) \
	$(NULL)

-include $(top_srcdir)/git.mk
