# trick Automake into defining $(CXXLINK)
if FALSE
BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc
endif

# I was so stupid
#gkbpath.h:
#	@echo -e '#define D_PATH "$(datadir)"' > gkbpath.h
#
# P_HEADER_1 = gkbpath.h

INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gkb_applet

gkb_applet_SOURCES = gkb.c properties.c properties.h

gkb_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) $(GNOMEGNORBA_LIBS)

# gkb_applet_DEPENDENCIES = $(P_HEADER_1)

gkb_applet_LINK = $(CXXLINK)

GKB_BITMAPS = be.xpm bg.xpm ca.xpm ch.xpm cz.xpm de.xpm dk.xpm es.xpm fi.xpm \
	fr.xpm gb.xpm gr.xpm hu.xpm il.xpm is.xpm it.xpm jp.xpm nl.xpm \
	no.xpm pl.xpm pt.xpm qc.xpm ru.xpm se.xpm si.xpm th.xpm tr.xpm \
	us.xpm yu.xpm

GKB_MAPS = xmodmap.be xmodmap.bg xmodmap.ch xmodmap.cz \
xmodmap.de xmodmap.dk xmodmap.dvorak xmodmap.es \
xmodmap.fi xmodmap.fr xmodmap.gr xmodmap.hu \
xmodmap.hu.old xmodmap.il xmodmap.is xmodmap.it \
xmodmap.la xmodmap.nl xmodmap.no xmodmap.pl \
xmodmap.pt xmodmap.qc xmodmap.ru xmodmap.ru_koi8 \
xmodmap.se xmodmap.sf xmodmap.sg xmodmap.si \
xmodmap.sk xmodmap.th xmodmap.tr_f xmodmap.tr_q \
xmodmap.uk xmodmap.us xmodmap.us.old xmodmap.yu


GKB_ICON = gkb.xpm

EXTRA_DIST = gkb_applet.desktop $(GKB_BITMAPS) $(GKB_ICON) $(GKB_MAPS)

sysdir = $(datadir)/applets/Utility
sys_DATA = gkb_applet.desktop

gkbbitmapsdir = $(datadir)/pixmaps/gkb
gkbbitmaps_DATA = $(GKB_BITMAPS)

gkbmapsdir = $(datadir)/xmodmap
gkbmaps_DATA = $(GKB_MAPS)

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)
