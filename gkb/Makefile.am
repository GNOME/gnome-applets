INCLUDES = -I. -I$(srcdir)                              \
        -I$(top_srcdir)/intl -I$(top_builddir)/intl     \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I$(includedir) $(GNOME_INCLUDEDIR)


bin_PROGRAMS = gkb_applet

SUBDIRS= gkb xmodmap docs

gkb_applet_SOURCES = gkb.c

gkb_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) $(INTLLIBS)

GKB_ICON = gkb.xpm

EXTRA_DIST = gkb_applet.desktop $(GKB_ICON) \
             gkb_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = gkb_applet.desktop

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gkb_applet.gnorba
