INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = clockmail_applet

clockmail_applet_SOURCES = \
	clockmail.c	\
	clockmail.h	\
	properties.c


clockmail_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

clockmail_applet_LINK = $(CXXLINK)

EXTRA_DIST = \
	clockmail_applet.desktop	\
	backgrnd.xpm			\
	digmed.xpm			\
	mailpics.xpm

sysdir = $(datadir)/applets/Utility
sys_DATA = clockmail_applet.desktop

