SUBDIRS = docs

desktop_iconsdir = $(iconsdir)/hicolor/48x48/apps
desktop_icons_DATA = gnome-mixer-applet.png

INCLUDES = -I. -I$(srcdir) 		\
	-DG_DISABLE_DEPRECATED		\
	-DGTK_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED	\
	-DGDK_DISABLE_DEPRECATED	\
	$(MIXER_CFLAGS)			\
	$(GNOME_APPLETS_CFLAGS)

libexec_PROGRAMS = mixer_applet2

mixer_applet2_SOURCES = \
	applet.c \
	dock.c \
	load.c \
	preferences.c

noinst_HEADERS = \
	applet.h \
	dock.h \
	keys.h \
	preferences.h

mixer_applet2_LDADD = 					\
	$(MIXER_LIBS) 					\
	$(GNOME_APPLETS_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MixerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

uidir   = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_MixerApplet.xml

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = mixer.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = 						\
	GNOME_MixerApplet.server.in.in			\
	$(ui_DATA) 					\
	$(desktop_icons_DATA)				\
	$(schemas_in_files)

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif
