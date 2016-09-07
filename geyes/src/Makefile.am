NULL =

uidir = $(pkgdatadir)/ui

geyes_appletlibdir = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)
geyes_appletlib_LTLIBRARIES = libgeyes-applet.la

libgeyes_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-I$(includedir) \
	-DGEYES_THEMES_DIR=\""$(pkgdatadir)/geyes/"\" \
	-DGEYES_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.geyes\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libgeyes_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libgeyes_applet_la_SOURCES = \
	geyes.c \
	geyes.h \
	themes.c \
	$(NULL)

libgeyes_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libgeyes_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS)\
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
