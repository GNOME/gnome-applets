INCLUDES = -I. -I$(srcdir)                              \
        -I$(top_srcdir)/intl -I$(top_builddir)/intl     \
        -I$(srcdir)/../../panel                         \
        -I$(top_builddir)/panel \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I$(includedir) $(GNOME_INCLUDEDIR)


#if DO_EXE
#bin_PROGRAMS = gkb_applet
#else

SUBDIRS= gkb xmodmap

lib_LTLIBRARIES=libgkb_applet.la

#endif

#bin_PROGRAMS = gkb_applet

libgkb_applet_la_SOURCES = gkb.c

# libgkb_applet_la_LDADD = ../../panel/libpanel_applet.la \
#		     $(GNOME_LIBDIR) $(ORB_LIBS) \
#		     $(GNOMEUI_LIBS) $(INTLLIBS) $(GNOMEGNORBA_LIBS)

# gkb_applet_DEPENDENCIES = $(P_HEADER_1)

# gkb_applet_LINK = $(CXXLINK)


GKB_ICON = gkb.xpm

EXTRA_DIST = gkb_applet.desktop $(GKB_ICON) \
             gkb_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = gkb_applet.desktop

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gkb_applet.gnorba
