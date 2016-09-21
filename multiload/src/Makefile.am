NULL =

uidir = $(pkgdatadir)/ui

multiload_appletlibdir = $(pkglibdir)
multiload_appletlib_LTLIBRARIES = libmultiload-applet.la

libmultiload_applet_la_CPPFLAGS = \
	-I$(srcdir) \
	-DMULTILOAD_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.multiload\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libmultiload_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
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
	main.c \
	netspeed.c \
	netspeed.h \
	properties.c \
	$(NULL)

libmultiload_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libmultiload_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GTOP_APPLETS_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
