INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = modemlights_applet

modemlights_applet_SOURCES = \
	modemlights.c	\
	modemlights.h	\
	properties.c

modemlights_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	modemlights_applet.desktop	\
	backgrnd.xpm			\
	backgrnd_s.xpm			\
	button_off.xpm			\
	button_on.xpm			\
	lights.xpm

sysdir = $(datadir)/applets/Network
sys_DATA = modemlights_applet.desktop

