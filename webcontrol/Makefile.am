INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = webcontrol_applet

webcontrol_applet_SOURCES = webcontrol.c

webcontrol_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

EXTRA_DIST = webcontrol_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = webcontrol_applet.desktop

