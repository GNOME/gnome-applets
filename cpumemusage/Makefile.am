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

bin_PROGRAMS = cpumemusage_applet

cpumemusage_applet_SOURCES = \
	cpumemusage.c	\
	proc.c		\
	proc.h		\
	procbar.c	\
	procbar.h


cpumemusage_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

cpumemusage_applet_LINK = $(CXXLINK)

EXTRA_DIST = cpumemusage_applet.desktop

sysdir = $(datadir)/applets/Monitors
sys_DATA = cpumemusage_applet.desktop
