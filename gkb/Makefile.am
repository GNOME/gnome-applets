INCLUDES = -I. -I$(srcdir)                              \
        -I$(top_srcdir)/intl -I$(top_builddir)/intl     \
        -I$(srcdir)/../../panel                         \
        -I$(top_builddir)/panel \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I$(includedir) $(GNOME_INCLUDEDIR)


#if DO_EXE
#bin_PROGRAMS = gkb_applet
#else

lib_LTLIBRARIES=libgkb_applet.la

#endif

#bin_PROGRAMS = gkb_applet

libgkb_applet_la_SOURCES = gkb.c

# libgkb_applet_la_LDADD = ../../panel/libpanel_applet.la \
#		     $(GNOME_LIBDIR) $(ORB_LIBS) \
#		     $(GNOMEUI_LIBS) $(INTLLIBS) $(GNOMEGNORBA_LIBS)

# gkb_applet_DEPENDENCIES = $(P_HEADER_1)

# gkb_applet_LINK = $(CXXLINK)

GKB_BITMAPS = 
	gkb/be.xpm gkb/bg.xpm gkb/ca.xpm gkb/ch.xpm \
	gkb/cz.xpm gkb/de.xpm gkb/dk.xpm gkb/es.xpm \
	gkb/fi.xpm gkb/fr.xpm gkb/gb.xpm gkb/gr.xpm \
	gkb/hu.xpm gkb/il.xpm gkb/is.xpm gkb/it.xpm \
	gkb/jp.xpm gkb/nl.xpm gkb/no.xpm gkb/pl.xpm \
	gkb/pt.xpm gkb/qc.xpm gkb/ru.xpm gkb/se.xpm \
	gkb/si.xpm gkb/th.xpm gkb/tr.xpm gkb/us.xpm \
	gkb/yu.xpm

GKB_MAPS = xmodmap/xmodmap.be xmodmap/xmodmap.bg xmodmap/xmodmap.ch xmodmap/xmodmap.cz \
	xmodmap/xmodmap.de xmodmap/xmodmap.dk xmodmap/xmodmap.dvorak xmodmap/xmodmap.es \
	xmodmap/xmodmap.fi xmodmap/xmodmap.fr xmodmap/xmodmap.gr xmodmap/xmodmap.hu \
	xmodmap/xmodmap.hu.old xmodmap/xmodmap.il xmodmap/xmodmap.is xmodmap/xmodmap.it \
	xmodmap/xmodmap.la xmodmap/xmodmap.nl xmodmap/xmodmap.no xmodmap/xmodmap.pl \
	xmodmap/xmodmap.pt xmodmap/xmodmap.qc xmodmap/xmodmap.ru xmodmap/xmodmap.ru_koi8 \
	xmodmap/xmodmap.se xmodmap/xmodmap.sf xmodmap/xmodmap.sg xmodmap/xmodmap.si \
	xmodmap/xmodmap.sk xmodmap/xmodmap.th xmodmap/xmodmap.tr_f xmodmap/xmodmap.tr_q \
	xmodmap/xmodmap.uk xmodmap/xmodmap.us xmodmap/xmodmap.us.old xmodmap/xmodmap.yu


GKB_ICON = gkb.xpm

EXTRA_DIST = gkb_applet.desktop $(GKB_BITMAPS) $(GKB_ICON) $(GKB_MAPS) \
             gkb_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = gkb_applet.desktop

gkbbitmapsdir = $(datadir)/pixmaps
gkbbitmaps_DATA = $(GKB_BITMAPS)

gkbmapsdir = $(datadir)
gkbmaps_DATA = $(GKB_MAPS)

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gkb_applet.gnorba
