INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = fifteen_applet

fifteen_applet_SOURCES = fifteen.c

fifteen_applet_LDADD = \
		       $(GNOME_LIBDIR) \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	fifteen_applet.desktop fifteen_applet.gnorba gnome-fifteen.png

sysdir = $(datadir)/applets/Amusements
sys_DATA = fifteen_applet.desktop

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-fifteen.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = fifteen_applet.gnorba
