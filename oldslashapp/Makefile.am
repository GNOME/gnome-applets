INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = slash_applet

slash_applet_SOURCES =  slashapp.c	\
			slashapp.h	\
			display.c	\
			properties.c

slash_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(GNOMEGNORBA_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS)      \
                     $(GHTTP_LIB)

EXTRA_DIST = \
	slash_applet.desktop back.xpm noimage.xpm slashsplash.xpm

sysdir = $(datadir)/applets/Amusements
sys_DATA = slash_applet.desktop

