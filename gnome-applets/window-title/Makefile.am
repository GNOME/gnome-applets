NULL =

noinst_LTLIBRARIES = libwindow-title-applet.la

libwindow_title_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.window-title\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libwindow_title_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libwindow_title_applet_la_SOURCES = \
	window-title-private.h \
	window-title.c \
	window-title.h \
	preferences.c \
	preferences.h \
	$(NULL)

libwindow_title_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libwindow_title_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(LIBWNCK_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
