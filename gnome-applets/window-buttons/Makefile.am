NULL =

SUBDIRS = themes

noinst_LTLIBRARIES = libwindow-buttons-applet.la

libwindow_buttons_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DWB_DATA_DIR=\""$(pkgdatadir)/window-buttons-applet"\" \
	$(NULL)

libwindow_buttons_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libwindow_buttons_applet_la_SOURCES = \
	window-buttons-private.h \
	window-buttons.c \
	window-buttons.h \
	preferences.c \
	preferences.h \
	external.c \
	external.h \
	theme.c \
	theme.h \
	$(NULL)

libwindow_buttons_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libwindow_buttons_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(LIBWNCK_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
