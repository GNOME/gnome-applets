INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = slash_applet

slash_applet_SOURCES =  slashapp.c	\
			slashapp.h	\
			display.c	\
			properties.c

slash_applet_LDADD = \
		     $(GNOME_LIBDIR) -lXt \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)      \
                     $(GHTTP_LIB) $(GNOME_XML_LIB)

EXTRA_DIST = \
	slash_applet.desktop slashsplash.xpm \
	slash_applet.gnorba noimage.xpm back.xpm \
	slashapp.png

sysdir = $(datadir)/applets/Network
sys_DATA = slash_applet.desktop

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = slashapp.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = slash_applet.gnorba
