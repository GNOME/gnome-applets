INCLUDES =                                           \
	-I.			                     \
	$(WARN_CFLAGS)                               \
	$(GNOME_APPLETS_CFLAGS)                                                            

bin_PROGRAMS = gkb-applet-2

bin_SCRIPTS = gkb_xmmap

gkb_applet_2_SOURCES = \
		gkb.c 		\
		gkb.h		\
		keygrab.c	\
		keymap.c	\
		prop.c 		\
		prop-list.c	\
		prop-map.c	\
		preset.c	\
		prop-add.c	\
		system.c	\
		util.c

gkb_applet_2_LDADD = \
	$(GNOME_APPLETS_LIBS) $(INTLLIBS)

@INTLTOOL_SERVER_RULE@

SUBDIRS= gkb xmodmap descs

GKB_ICON = gkb.png gkb-icon.png

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gkb.schemas

appletdir = $(libdir)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_KeyboardApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

EXTRA_DIST = $(GKB_ICON) \
	$(server_DATA) $(server_in_files) $(schema_DATA)

gkb_xmmap: gkb_xmmap.in
	sed "s,\$${prefix},$(prefix),g" \
	< gkb_xmmap.in >gkb_xmmap.tmp \
	&& mv gkb_xmmap.tmp gkb_xmmap
