INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 

bin_PROGRAMS = screenshooter-applet2

screenshooter_applet2_SOURCES = \
	screenshooter_applet.c		\
	screenshooter_applet.h		\
	root_icon.xpm  \
	window_icon.xpm 

screenshooter_applet2_LDADD = \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

scrshotdir = $(bindir)
scrshot_SCRIPTS = update_scrshot_page

pngdir = $(datadir)/pixmaps
png_DATA = screenshooter_applet.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_ScreenshooterApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = $(png_DATA) $(scrshot_SCRIPTS)	\
	$(server_in_files) 

#SUBDIRS = docs
