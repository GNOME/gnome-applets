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

bin_PROGRAMS = netload_applet

netload_applet_SOURCES = \
	netload.c 	\
	netload.h	\
	linux-proc.h 	\
	linux-proc.c 	\
	properties.c	\
	properties.h

netload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

netload_applet_LINK = $(CXXLINK)

EXTRA_DIST = netload_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = netload_applet.desktop
