#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(GNOME_APPLETS_CFLAGS)				\
	-I$(includedir)

bin_PROGRAMS = mixer_applet2

mixer_applet2_SOURCES = 				\
	mixer.c

mixer_applet2_LDADD = 					\
	$(MIXER_LIBS) 					\
	$(GNOME_APPLETS_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MixerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = 						\
	$(server_DATA) 					\
	$(server_in_files)				\
	volume-max.xpm					\
	volume-medium.xpm				\
	volume-min.xpm					\
	volume-mute.xpm					\
	volume-zero.xpm
