INCLUDES = -I. -I$(srcdir)                              \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl     \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	@PIXBUF_CFLAGS@ -DGNOMEDATADIR=\""$(datadir)"\"

bin_PROGRAMS = gkb_applet

bin_SCRIPTS = gkb_xmmap

gkb_applet_SOURCES = gkb.c gkb.h prop.c

#lib_LTLIBRARIES=libgkb_applet.la

#libgkb_applet_la_SOURCES= gkb.c gkb.h prop.c	

gkb_applet_LDADD =     $(GNOME_APPLETS_LIBS) \
                       $(GNOME_LIBDIR) \
                       $(GNOMEGNORBA_LIBS) $(INTLLIBS) \
                       @PIXBUF_LIBS@

SUBDIRS= gkb 

GKB_ICON = gkb.png gkb-icon.png

EXTRA_DIST = gkb_applet.desktop $(GKB_ICON) \
	gkb_applet.gnorba 

sysdir = $(datadir)/applets/Utility
sys_DATA = gkb_applet.desktop

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gkb_applet.gnorba

gkb_xmmap: gkb_xmmap.in ../config.status
	sed "s,\$${prefix},$(prefix),g" \
        < gkb_xmmap.in >gkb_xmmap.tmp \
	&& mv gkb_xmmap.tmp gkb_xmmap
