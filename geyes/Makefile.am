SUBDIRS = themes docs

INCLUDES =						\
	-I.						\
	-I$(srcdir) 					\
	$(GNOME_APPLETS_CFLAGS)				\
	-I$(includedir) 				\
	-DGEYES_THEMES_DIR=\""$(datadir)/geyes/"\"

libexec_PROGRAMS = geyes_applet2

geyes_applet2_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet2_LDADD = \
	$(GNOME_APPLETS_LIBS)


serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GeyesApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GeyesApplet.xml

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = geyes.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

@INTLTOOL_SERVER_RULE@

themesdir = $(datadir)/geyes

pixmapdir = $(datadir)/icons/hicolor/48x48/apps
pixmap_DATA = gnome-eyes.png

EXTRA_DIST = \
	     geyes.h \
	     gnome-eyes.png \
	     README.themes \
	     $(pixmap_DATA) \
	     GNOME_GeyesApplet.server.in.in \
	     $(schemas_in_files) \
	     $(ui_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif
