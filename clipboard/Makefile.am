INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = clipboard_applet

clipboard_applet_SOURCES = clipboard.c

clipboard_applet_LDADD = \
		       $(GNOME_LIBDIR) \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Utility

sys_predata = clipboard_applet.desktop.in

sys_DATA = $(sys_predata:.desktop.in=.desktop)

@XML_I18N_MERGE_DESKTOP_RULE@

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-clipboard.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = clipboard_applet.gnorba

EXTRA_DIST = \
	copy.xpm set_selection.xpm \
	gnome-clipboard.png	\
	$(gnorba_DATA)	\
	$(sys_DATA) $(sys_predata)
