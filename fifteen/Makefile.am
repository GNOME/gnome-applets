#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 

bin_PROGRAMS = fifteen_applet2

fifteen_applet2_SOURCES = fifteen.c

fifteen_applet2_LDADD = \
		       $(GNOME_APPLETS_LIBS)



pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-fifteen.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_FifteenApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = \
	$(server_in_files) $(pixmap_DATA)	

