SUBDIRS = themes docs

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
sys_predata = odometer_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = odometer_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-odometer.png

man_MANS = odometer_applet.1

EXTRA_DIST = \
	README.themes	\
	$(pixmap_DATA)	\
	$(gnorba_DATA)	\
	$(sys_DATA)	\
	$(sys_predata)	\
	$(man_MANS)
	
