INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = wmpager_applet

wmpager_applet_SOURCES = \
	wmpager_applet.c	\
	WinMgr.h

wmpager_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = wmpager_applet.desktop wmpager_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = wmpager_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = wmpager_applet.gnorba
