INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc

bin_PROGRAMS = netload_applet

netload_applet_SOURCES = netload.c 	\
			 linux-proc.h 	\
			 linux-proc.c 	\
			 properties.c	\
			 properties.h
MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

netload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

netload_applet_LINK = $(CXXLINK)

EXTRA_DIST = netload_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = netload_applet.desktop
