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
	pixmaps/be.xpm pixmaps/bg.xpm pixmaps/ca.xpm pixmaps/ch.xpm \
	pixmaps/cz.xpm pixmaps/de.xpm pixmaps/dk.xpm pixmaps/es.xpm \
	pixmaps/fi.xpm pixmaps/fr.xpm pixmaps/gb.xpm pixmaps/gr.xpm \
	pixmaps/hu.xpm pixmaps/il.xpm pixmaps/is.xpm pixmaps/it.xpm \
	pixmaps/jp.xpm pixmaps/nl.xpm pixmaps/no.xpm pixmaps/pl.xpm \
	pixmaps/pt.xpm pixmaps/qc.xpm pixmaps/ru.xpm pixmaps/se.xpm \
	pixmaps/si.xpm pixmaps/th.xpm pixmaps/tr.xpm pixmaps/us.xpm \
	pixmaps/yu.xpm

GKB_MAPS = xmodmaps/xmodmap.be xmodmaps/xmodmap.bg xmodmaps/xmodmap.ch xmodmaps/xmodmap.cz \
	xmodmaps/xmodmap.de xmodmaps/xmodmap.dk xmodmaps/xmodmap.dvorak xmodmaps/xmodmap.es \
	xmodmaps/xmodmap.fi xmodmaps/xmodmap.fr xmodmaps/xmodmap.gr xmodmaps/xmodmap.hu \
	xmodmaps/xmodmap.hu.old xmodmaps/xmodmap.il xmodmaps/xmodmap.is xmodmaps/xmodmap.it \
	xmodmaps/xmodmap.la xmodmaps/xmodmap.nl xmodmaps/xmodmap.no xmodmaps/xmodmap.pl \
	xmodmaps/xmodmap.pt xmodmaps/xmodmap.qc xmodmaps/xmodmap.ru xmodmaps/xmodmap.ru_koi8 \
	xmodmaps/xmodmap.se xmodmaps/xmodmap.sf xmodmaps/xmodmap.sg xmodmaps/xmodmap.si \
	xmodmaps/xmodmap.sk xmodmaps/xmodmap.th xmodmaps/xmodmap.tr_f xmodmaps/xmodmap.tr_q \
	xmodmaps/xmodmap.uk xmodmaps/xmodmap.us xmodmaps/xmodmap.us.old xmodmaps/xmodmap.yu


GKB_ICON = gkb.xpm

EXTRA_DIST = gkb_applet.desktop $(GKB_BITMAPS) $(GKB_ICON) $(GKB_MAPS) \
             gkb_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = gkb_applet.desktop

gkbbitmapsdir = $(datadir)/pixmaps/gkb
gkbbitmaps_DATA = $(GKB_BITMAPS)

gkbmapsdir = $(datadir)/xmodmap
gkbmaps_DATA = $(GKB_MAPS)

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gkb_applet.gnorba
