INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	$(LIBGTOP_INCS)

bin_PROGRAMS = netload_applet

netload_applet_SOURCES = \
	netload.c 	\
	netload.h	\
	linux-proc.h 	\
	linux-proc.c 	\
	properties.c	\
	properties.h

netload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST = netload_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = netload_applet.desktop
