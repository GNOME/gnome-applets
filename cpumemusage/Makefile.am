INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-I$(LIBGTOP_INCS)

bin_PROGRAMS = cpumemusage_applet

cpumemusage_applet_SOURCES = \
	cpumemusage.c	\
	proc.c		\
	proc.h		\
	procbar.c	\
	procbar.h


cpumemusage_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

cpumemusage_applet_LINK = $(CXXLINK)

EXTRA_DIST = cpumemusage_applet.desktop

sysdir = $(datadir)/applets/Monitors
sys_DATA = cpumemusage_applet.desktop
