INCLUDES =                                           \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"      \
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
		util.c		\
		gconf.c


gkb_applet_2_LDADD = \
	$(GNOME_APPLETS_LIBS) $(X_LIBS) $(INTLLIBS)

@INTLTOOL_SERVER_RULE@

if GKB_SUN
gkb_DDIRS= xmodmap.sun descs.sun
else
gkb_DDIRS= xmodmap descs
endif

SUBDIRS= gkb help $(gkb_DDIRS)

GKB_ICON = gkb.png gkb-icon.png

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gkb.schemas

appletdir = $(libdir)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_KeyboardApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_KeyboardApplet.xml

EXTRA_DIST = $(GKB_ICON) \
	$(server_DATA) $(server_in_files) $(schema_DATA) $(ui_DATA)

gkb_xmmap: gkb_xmmap.in
	sed "s,\$${prefix},$(prefix),g" \
	< gkb_xmmap.in >gkb_xmmap.tmp \
	&& mv gkb_xmmap.tmp gkb_xmmap

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;

