INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc

bin_PROGRAMS = clock_applet

clock_applet_SOURCES = clock.c

MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

clock_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

clock_applet_LINK = $(CXXLINK)

EXTRA_DIST = clock_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = clock_applet.desktop

