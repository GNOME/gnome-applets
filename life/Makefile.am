SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = life_applet

life_applet_SOURCES = life.c

life_applet_LDADD = \
		       $(GNOME_LIBDIR) \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	life_applet.desktop life_applet.gnorba \
	gnome-life.png

sysdir = $(datadir)/applets/Amusements
sys_DATA = life_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = life_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-life.png
