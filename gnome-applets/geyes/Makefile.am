NULL =

SUBDIRS = themes

noinst_LTLIBRARIES = libgeyes-applet.la

libgeyes_applet_la_CPPFLAGS = \
	-DGEYES_THEMES_DIR=\""$(pkgdatadir)/geyes/"\" \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.geyes\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libgeyes_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libgeyes_applet_la_SOURCES = \
	geyes-applet-private.h \
	geyes-applet.c \
	geyes-applet.h \
	themes.c \
	$(NULL)

libgeyes_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libgeyes_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS)\
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
