# trick Automake into defining $(CXXLINK)
if FALSE
BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc
endif

INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gkb_applet

gkb_applet_SOURCES = gkb.c properties.c properties.h

gkb_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

gkb_applet_LINK = $(CXXLINK)

GKB_BITMAPS = hu.xpm us.xpm gb.xpm fr.xpm de.xpm

GKB_ICON = gkb.xpm

EXTRA_DIST = gkb_applet.desktop $(GKB_BITMAPS) $(GKB_ICON)

sysdir = $(datadir)/applets/Utility
sys_DATA = gkb_applet.desktop

gkbbitmapsdir = $(datadir)/pixmaps/gkb
gkbbitmaps_DATA = $(GKB_BITMAPS)

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)
