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

bin_PROGRAMS = winlist_applet

winlist_applet_SOURCES = \
	winlist.c	\
	winlist.h	\
	fvwm-winlist.c	\
	fvwmlib.h	\
	module.h


winlist_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

winlist_applet_LINK = $(CXXLINK)

EXTRA_DIST = winlist_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = winlist_applet.desktop

