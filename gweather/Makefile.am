## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps docs

INCLUDES = \
	-I$(srcdir)	-I$(srcdir)/pixmaps	\
	-I$(top_srcdir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)		\
	$(GNOME_INCLUDEDIR)	\
	$(PIXBUF_CFLAGS)

bin_PROGRAMS = gweather

gweather_SOURCES = \
	main.c \
	gweather-about.c gweather-about.h \
	gweather-pref.c gweather-pref.h \
	gweather-dialog.c gweather-dialog.h \
	gweather-applet.c gweather-applet.h \
	weather.c weather.h \
	http.c http.h

gweather_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS)	\
	$(INTLLIBS) \
	$(PIXBUF_LIBS) \
	$(GHTTP_LIB)


gweathergnorbadir = $(sysconfdir)/CORBA/servers
gweathergnorba_DATA = gweather.gnorba

gweatherappletdir = $(datadir)/applets/Utility
gweatherapplet_predata = gweather.desktop.in
gweatherapplet_DATA = $(gweatherapplet_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gweatherlocationsdir = $(datadir)/gweather
gweatherlocations_DATA = Locations

man_MANS = gweather_applet.1

EXTRA_DIST = \
	Locations			\
	$(gweathergnorba_DATA)		\
	$(gweatherapplet_DATA)		\
	$(gweatherapplet_predata)	\
	$(man_MANS)

