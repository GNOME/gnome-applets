NULL =

SUBDIRS = \
	brightness \
	command \
	gweather \
	timer \
	trash \
	window-picker \
	$(NULL)

gnome_applets_libdir = $(GNOME_PANEL_MODULES_DIR)
gnome_applets_lib_LTLIBRARIES = org.gnome.gnome-applets.la

org_gnome_gnome_applets_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\""org.gnome.gnome-applets"\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DLOCALE_DIR=\"$(localedir)\" \
	$(AM_CPPFLAGS) \
	$(NULL)

org_gnome_gnome_applets_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBGWEATHER_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

org_gnome_gnome_applets_la_SOURCES = \
	ga-module.c \
	$(NULL)

org_gnome_gnome_applets_la_LIBADD = \
	$(top_builddir)/gnome-applets/brightness/libbrightness-applet.la \
	$(top_builddir)/gnome-applets/command/libcommand-applet.la \
	$(top_builddir)/gnome-applets/gweather/libgweather-applet.la \
	$(top_builddir)/gnome-applets/timer/libtimer-applet.la \
	$(top_builddir)/gnome-applets/trash/libtrash-applet.la \
	$(top_builddir)/gnome-applets/window-picker/libwindow-picker-applet.la \
	$(GNOME_PANEL_LIBS) \
	$(NULL)

org_gnome_gnome_applets_la_LDFLAGS = \
	-module -avoid-version \
	$(AM_LDFLAGS) \
	$(NULL)

-include $(top_srcdir)/git.mk
