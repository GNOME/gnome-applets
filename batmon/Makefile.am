pixmapdir = $(datadir)/pixmaps

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = batmon_applet

batmon_applet_SOURCES = batmon.c

batmon_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

EXTRA_DIST = batmon_applet.desktop batmon-ac.xpm batmon-bat.xpm

sysdir = $(datadir)/applets/Monitors
sys_DATA = batmon_applet.desktop

pixmap_DATA = batmon-ac.xpm batmon-bat.xpm
