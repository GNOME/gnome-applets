NULL =

uidir = $(pkgdatadir)/ui

libexec_PROGRAMS = geyes_applet2

geyes_applet2_CFLAGS = \
	-I. \
	-I$(srcdir) \
	$(GNOME_APPLETS_CFLAGS) \
	-I$(includedir) \
	-DGEYES_THEMES_DIR=\""$(pkgdatadir)/geyes/"\" \
	-DGEYES_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

geyes_applet2_SOURCES = \
	geyes.c \
	themes.c \
	geyes.h \
	$(NULL)

geyes_applet2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

geyes_applet2_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBM) \
	$(NULL)

-include $(top_srcdir)/git.mk
