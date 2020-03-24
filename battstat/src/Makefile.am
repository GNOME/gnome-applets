NULL =

uidir = $(pkgdatadir)/ui

battery_status_applet_libdir = $(pkglibdir)
battery_status_applet_lib_LTLIBRARIES = libbattery-status-applet.la

libbattery_status_applet_la_CPPFLAGS = \
	-DBATTSTAT_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.battery-status\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libbattery_status_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(UPOWER_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libbattery_status_applet_la_SOURCES = \
	battstat.h \
	pixmaps.h \
	properties.c \
	battstat_applet.c \
	battstat-upower.c \
	battstat-upower.h \
	$(NULL)

libbattery_status_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libbattery_status_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(UPOWER_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
