INCLUDES = -I. -I$(srcdir) 					\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-I$(includedir) $(GNOME_INCLUDEDIR)			\
	-DODOMETER_THEMES_DIR=\""$(datadir)/odometer/"\"

bin_PROGRAMS = odometer_applet

odometer_applet_SOURCES = odo.c odo.h properties.c theme.c

odometer_applet_LDADD = \
		$(GNOME_LIBDIR) \
		$(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Amusements
sys_DATA = odometer_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = odometer_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-odometer.png

EXTRA_DIST = \
	README.themes	\
	$(sys_DATA)	\
	$(gnorba_DATA)	\
	$(pixmap_DATA)

SUBDIRS = themes
