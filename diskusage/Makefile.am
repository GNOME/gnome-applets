INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc

bin_PROGRAMS = diskusage_applet

diskusage_applet_SOURCES = diskusage.c diskusage_read.c procbar.c \
	fsusage.c fsusage.h mountlist.c mountlist.h properties.c

MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

diskusage_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

diskusage_applet_LINK = $(CXXLINK)

EXTRA_DIST = diskusage_applet.desktop

sysdir = $(datadir)/applets/Monitors
sys_DATA = diskusage_applet.desktop
