INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc

bin_PROGRAMS = cpumemusage_applet

cpumemusage_applet_SOURCES = cpumemusage.c proc.c procbar.c

MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

cpumemusage_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS) -lcrypt

cpumemusage_applet_LINK = $(CXXLINK)

EXTRA_DIST = cpumemusage_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = cpumemusage_applet.desktop
