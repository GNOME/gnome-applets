INCLUDES =                                           \
	-I$(top_srcdir)/screen-exec			\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"      \
	$(GNOME_APPLETS_CFLAGS)                                                            

bin_PROGRAMS = gnome-keyboard-layout
bin_SCRIPTS = gkb_xmmap
noinst_LIBRARIES = libgkb.a

gkb_COMMON = \
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

gnome_keyboard_layout_SOURCES	= $(gkb_COMMON) gnome-keyboard-layout.c
gnome_keyboard_layout_LDADD	= \
	$(top_builddir)/screen-exec/libscreen-exec.la \
	$(GNOME_APPLETS_LIBS) $(X_LIBS) $(INTLLIBS)

libgkb_a_SOURCES		= $(gkb_COMMON) gkb-applet.c

if GKB_SUN
gkb_DDIRS= xmodmap.sun descs.sun
else
gkb_DDIRS= xmodmap descs
endif

SUBDIRS= gkb help $(gkb_DDIRS)

GKB_ICON = gkb.png gkb-icon.png

gkbicondir = $(datadir)/pixmaps
gkbicon_DATA = $(GKB_ICON)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = gkb.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_KeyboardApplet.xml

EXTRA_DIST =						\
	$(GKB_ICON)					\
	$(schemas_in_files)					\
	$(ui_DATA)

gkb_xmmap: gkb_xmmap.in
	sed "s,\$${prefix},$(prefix),g" \
	< gkb_xmmap.in >gkb_xmmap.tmp \
	&& mv gkb_xmmap.tmp gkb_xmmap

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif
