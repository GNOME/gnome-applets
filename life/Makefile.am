#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = life_applet2

life_applet2_SOURCES = life.c

life_applet2_LDADD = \
		       $(GNOME_APPLETS_LIBS)

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-life.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_LifeApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@
EXTRA_DIST = $(pixmap_DATA)	\
	$(server_in_files)
