INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = wmpager_applet

wmpager_applet_SOURCES = \
	wmpager_applet.c	\
	WinMgr.h


wmpager_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

wmpager_applet_LINK = $(CXXLINK)

EXTRA_DIST = wmpager_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = wmpager_applet.desktop

