pixmapdir = $(datadir)/pixmaps

INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = batmon_applet

batmon_applet_SOURCES = batmon.c

MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

batmon_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

# some of the sources/libs use C++
batmon_applet_LINK = $(CXXLINK)

EXTRA_DIST = batmon_applet.desktop batmon-ac.xpm batmon-bat.xpm

sysdir = $(datadir)/applets/Monitors
sys_DATA = batmon_applet.desktop

pixmap_DATA = batmon-ac.xpm batmon-bat.xpm
