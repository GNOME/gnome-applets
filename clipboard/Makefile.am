INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = clipboard_applet

clipboard_applet_SOURCES = clipboard.c

clipboard_applet_LDADD = \
		       $(GNOME_LIBDIR) \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	clipboard_applet.desktop	\
	clipboard_applet.gnorba		\
	copy.xpm set_selection.xpm \
	gnome-clipboard.png

sysdir = $(datadir)/applets/Utility
sys_DATA = clipboard_applet.desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-clipboard.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = clipboard_applet.gnorba
