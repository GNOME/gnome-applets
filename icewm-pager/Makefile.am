# trick Automake into defining $(CXXLINK)
if FALSE
BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc
endif

INCLUDES = -I. -I$(srcdir) 				\
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

