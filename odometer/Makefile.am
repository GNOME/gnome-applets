INCLUDES = -I. -I$(srcdir) 					\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-I$(includedir) $(GNOME_INCLUDEDIR)			\
	-DODOMETER_THEMES_DIR=\""$(datadir)/pixmaps/odometer/"\"

bin_PROGRAMS = odometer_applet

odometer_applet_SOURCES = odo.c odo.h properties.c B.xpm

odometer_applet_LDADD = \
		$(GNOME_LIBDIR) \
		$(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Amusements
sys_DATA = odometer_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = odometer_applet.gnorba

themedir = $(datadir)/pixmaps/odometer
theme_DATA = 57chevy.xpm A.xpm bbldotg.xpm fp2.xpm mpurple.xpm o.xpm smplst.xpm

EXTRA_DIST = README.themes $(sys_DATA) $(gnorba_DATA) $(theme_DATA)
