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

bin_PROGRAMS = cpuload_applet

cpuload_applet_SOURCES = cpuload.c 	\
			 linux-proc.h 	\
			 linux-proc.c 	\
			 properties.c	\
			 properties.h
MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

cpuload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

cpuload_applet_LINK = $(CXXLINK)

EXTRA_DIST = cpuload_applet.desktop

sysdir = $(datadir)/applets/Monitors
sys_DATA = cpuload_applet.desktop
