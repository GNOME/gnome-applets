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

bin_PROGRAMS = diskusage_applet

diskusage_applet_SOURCES = \
	diskusage.c		\
	diskusage.h		\
	diskusage_read.c	\
	fsusage.c		\
	fsusage.h		\
	mountlist.c		\
	mountlist.h		\
	properties.c		\
	properties.h


diskusage_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

diskusage_applet_LINK = $(CXXLINK)

EXTRA_DIST = diskusage_applet.desktop

sysdir = $(datadir)/applets/Monitors
sys_DATA = diskusage_applet.desktop
