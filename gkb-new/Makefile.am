INCLUDES =                                           \
	-I.			                     \
	$(WARN_CFLAGS)                               \
	$(GNOME_APPLETS_CFLAGS)                                                            

bin_PROGRAMS = gkb-applet-2

gkb_applet_2_SOURCES = \
		gkb.c 		\
		gkb.h		\
		keygrab.c	\
		keymap.c	\
		prop.c 		\
		prop-list.c	\
		prop-map.c	\
		preset.c	\
		system.c	\
		util.c

gkb_applet_2_LDADD = \
	$(GNOME_APPLETS_LIBS) $(INTLLIBS)

# @INTLTOOL_SERVER_RULE@

SUBDIRS= gkb xmodmap descs

GKB_ICON = gkb.png gkb-icon.png

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

appletdir = $(libdir)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_KeyboardApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

EXTRA_DIST = $(GKB_ICON) \
	$(server_DATA)
