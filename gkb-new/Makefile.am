INCLUDES = -I. -I$(srcdir)                              \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl     \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	@PIXBUF_CFLAGS@ -DGNOMEDATADIR=\""$(datadir)"\"

bin_PROGRAMS = gkb_applet

bin_SCRIPTS = gkb_xmmap

gkb_applet_SOURCES = \
		gkb.c 		\
		gkb.h		\
		keygrab.c		\
		keymap.c		\
		prop.c 		\
		prop-add.c	\
		prop-list.c	\
		prop-map.c	\
		preset.c		\
		system.c		\
		util.c

#lib_LTLIBRARIES=libgkb_applet.la

#libgkb_applet_la_SOURCES= gkb.c gkb.h prop.c preset.c

gkb_applet_LDADD =     $(GNOME_APPLETS_LIBS) \
                       $(GNOME_LIBDIR) \
                       $(GNOMEGNORBA_LIBS) $(INTLLIBS) \
                       @PIXBUF_LIBS@

SUBDIRS= gkb xmodmap descs docs

GKB_ICON = gkb.png gkb-icon.png

sysdir = $(datadir)/applets/Utility
sys_predata = gkb_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gkb_applet.gnorba

man_MANS = gkb_applet.1

EXTRA_DIST = $(GKB_ICON) \
	$(gnorba_DATA)		\
	$(sys_DATA)		\
	$(sys_predata)		\
	$(man_MANS)


gkb_xmmap: gkb_xmmap.in ../config.status
	sed "s,\$${prefix},$(prefix),g" \
        < gkb_xmmap.in >gkb_xmmap.tmp \
	&& mv gkb_xmmap.tmp gkb_xmmap
