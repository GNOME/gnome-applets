INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = screenshooter_applet

screenshooter_applet_SOURCES = \
	screenshooter_applet.c

screenshooter_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Utility
sys_DATA = screenshooter_applet.desktop

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = screenshooter_applet.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = screenshooter_applet.gnorba

#scriptdir = $(datadir)/screenshooter_applet
script = update_scrshot_page

extra_icons = root_icon.xpm window_icon.xpm

EXTRA_DIST = \
	$(sys_DATA) $(pixmap_DATA) $(gnorba_DATA) $(script_DATA) $(extra_icons)


