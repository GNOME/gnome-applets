INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc

bin_PROGRAMS = webcontrol_applet

webcontrol_applet_SOURCES = webcontrol.c

MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

webcontrol_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

webcontrol_applet_LINK = $(CXXLINK)

EXTRA_DIST = webcontrol_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = webcontrol_applet.desktop

