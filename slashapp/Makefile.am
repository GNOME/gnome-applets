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

sysdir = $(datadir)/applets/Network
sys_predata = slash_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = slashapp.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = slash_applet.gnorba

man_MANS = slash_applet.1

EXTRA_DIST = \
	slashsplash.xpm gnotices.xpm \
	noimage.xpm back.xpm \
	slashapp.png	\
	$(gnorba_DATA)	\
	$(sys_DATA)	\
	$(sys_predata)	\
	$(man_MANS)
	

