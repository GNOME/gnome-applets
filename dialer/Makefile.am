INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = dialer_applet

dialer_applet_SOURCES =	\
        dialer.c	\
	dialer.h


dialer_applet_LDADD = ../../panel/libpanel_applet.la		\
		     $(GNOME_LIBDIR) $(ORB_LIBS)     		\
		     $(GNOMEUI_LIBS) $(INTLLIBS)

dialer_applet_LINK = $(CXXLINK)

EXTRA_DIST = dialer_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = dialer_applet.desktop

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images
