NULL =

noinst_LTLIBRARIES = libtimer-applet.la

libtimer_applet_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.timer\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libtimer_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtimer_applet_la_SOURCES = \
	timer-applet.c \
	timer-applet.h \
	$(NULL)

libtimer_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libtimer_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
