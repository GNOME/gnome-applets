INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gticker_applet

gticker_applet_SOURCES =  gticker.c	\
			gticker.h	\
			display.c	\
			properties.c

gticker_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS) -lghttp 

EXTRA_DIST = \
	gticker_applet.desktop back.xpm noimage.xpm gticker.xpm

sysdir = $(datadir)/applets/Amusements
sys_DATA = gticker_applet.desktop

