#SUBDIRS = themes docs
SUBDIRS = themes

INCLUDES = -I. -I$(srcdir)                              \
        $(GNOME_APPLETS_CFLAGS)                         \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I$(includedir)                                 \
        -DODOMETER_THEMES_DIR=\""$(datadir)/odometer/"\"

bin_PROGRAMS = odometer_applet2

odometer_applet2_SOURCES = odo.c odo.h properties.c theme.c

odometer_applet2_LDADD = \
		$(GNOME_APPLETS_LIBS)


pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-odometer.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_OdometerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@	

EXTRA_DIST = \
	README.themes	\
	$(pixmap_DATA)	\
	$(server_in_files)
