INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-I$(LIBGTOP_INCS)

bin_PROGRAMS = cpuload_applet

cpuload_applet_SOURCES = cpuload.c 	\
			 linux-proc.h 	\
			 linux-proc.c 	\
			 properties.c	\
			 properties.h

cpuload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

cpuload_applet_LINK = $(CXXLINK)

EXTRA_DIST = cpuload_applet.desktop

sysdir = $(datadir)/applets/Monitors
sys_DATA = cpuload_applet.desktop
