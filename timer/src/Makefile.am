NULL =

timer_applet_libdir = $(pkglibdir)
timer_applet_lib_LTLIBRARIES = libtimer-applet.la

libtimer_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.timer\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	$(NULL)

libtimer_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtimer_applet_la_SOURCES = \
	timer-applet.c \
	$(NULL)

libtimer_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libtimer_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
