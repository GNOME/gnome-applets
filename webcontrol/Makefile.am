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

bin_PROGRAMS = webcontrol_applet

webcontrol_applet_SOURCES = webcontrol.c


webcontrol_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

webcontrol_applet_LINK = $(CXXLINK)

EXTRA_DIST = webcontrol_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = webcontrol_applet.desktop

