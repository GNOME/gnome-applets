NULL =

uidir = $(pkgdatadir)/ui

libexec_PROGRAMS = multiload-applet-2

multiload_applet_2_CFLAGS = \
	-I$(srcdir) \
	-DMULTILOAD_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(GTOP_APPLETS_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

multiload_applet_2_SOURCES = \
	global.h \
	linux-proc.h \
	load-graph.h \
	linux-proc.c \
	load-graph.c \
	main.c \
	properties.c \
	netspeed.c netspeed.h \
	autoscaler.c \
	autoscaler.h \
	$(NULL)

multiload_applet_2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

multiload_applet_2_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GTOP_APPLETS_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
