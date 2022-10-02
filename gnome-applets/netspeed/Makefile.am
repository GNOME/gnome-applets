NULL =

noinst_LTLIBRARIES = libnet-speed-applet.la

libnet_speed_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.net-speed\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-DTIME_WITH_SYS_TIME=1 \
	-DHAVE_SYS_TIME_H=1 \
	-I$(top_srcdir) \
	$(NULL)

libnet_speed_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(GTOP_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libnet_speed_applet_la_SOURCES = \
	backend.h \
	backend.c \
	label.h \
	label.c \
	netspeed-applet.c \
	netspeed-applet.h \
	preferences.h \
	preferences.c \
	$(NULL)

libnet_speed_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libnet_speed_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(GTOP_APPLETS_LIBS) \
	$(IWLIB) \
	$(NULL)

-include $(top_srcdir)/git.mk
