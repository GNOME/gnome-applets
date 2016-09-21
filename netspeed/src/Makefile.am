NULL =

uidir = $(pkgdatadir)/ui

netspeed_applet_libdir = $(pkglibdir)
netspeed_applet_lib_LTLIBRARIES = libnet-speed-applet.la

libnet_speed_applet_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.net-speed\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DNETSPEED_MENU_UI_DIR=\""$(uidir)"\" \
	-I$(top_srcdir) \
	-I$(includedir) \
	$(NULL)

libnet_speed_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(GTOP_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libnet_speed_applet_la_SOURCES = \
	backend.h \
	backend.c \
	label.h \
	label.c \
	netspeed.h \
	netspeed.c \
	preferences.h \
	preferences.c \
	$(NULL)

libnet_speed_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libnet_speed_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GTOP_APPLETS_LIBS) \
	$(IWLIB) \
	$(NULL)

-include $(top_srcdir)/git.mk
