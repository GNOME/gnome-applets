NULL =

noinst_LTLIBRARIES = libmultiload-applet.la

libmultiload_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.multiload\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DTIME_WITH_SYS_TIME=1 \
	-DHAVE_SYS_TIME_H=1 \
	$(NULL)

libmultiload_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(GTOP_APPLETS_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libmultiload_applet_la_SOURCES = \
	autoscaler.c \
	autoscaler.h \
	global.h \
	linux-proc.c \
	linux-proc.h \
	load-graph.h \
	load-graph.c \
	ma-disk-stats.c \
	ma-disk-stats.h \
	multiload-applet.c \
	multiload-applet.h \
	netspeed.c \
	netspeed.h \
	properties.c \
	$(NULL)

libmultiload_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libmultiload_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(GTOP_APPLETS_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
