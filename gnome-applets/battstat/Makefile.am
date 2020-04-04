NULL =

noinst_LTLIBRARIES = libbattery-status-applet.la

libbattery_status_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.battery-status\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libbattery_status_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(UPOWER_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libbattery_status_applet_la_SOURCES = \
	battstat.h \
	pixmaps.h \
	properties.c \
	battstat-applet.c \
	battstat-applet.h \
	battstat-upower.c \
	battstat-upower.h \
	$(NULL)

libbattery_status_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libbattery_status_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(UPOWER_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
