#SUBDIRS = docs

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-mixer-applet.png

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED \
	$(GNOME_APPLETS_CFLAGS)				

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

uidir   = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_MixerApplet.xml

EXTRA_DIST = 						\
	$(server_DATA) 					\
	$(server_in_files)				\
	$(ui_DATA) 					\
	volume-max.xpm					\
	volume-medium.xpm				\
	volume-min.xpm					\
	volume-mute.xpm					\
	volume-zero.xpm					\
	gnome-mixer-applet.png
