SUBDIRS = themes docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/screen-exec			\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-I$(includedir) 				\
	-DGEYES_THEMES_DIR=\""$(datadir)/geyes/"\"

libexec_PROGRAMS = geyes_applet2

geyes_applet2_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet2_LDADD = \
	$(top_builddir)/screen-exec/libscreen-exec.la \
	$(GNOME_APPLETS_LIBS)


serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GeyesApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GeyesApplet.xml

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = geyes.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

@INTLTOOL_SERVER_RULE@

themesdir = $(datadir)/geyes

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-eyes.png

EXTRA_DIST = \
	     geyes.h \
	     gnome-eyes.png \
	     README.themes \
	     $(pixmap_DATA) \
	     GNOME_GeyesApplet.server.in.in \
	     $(schemas_in_files) \
	     $(ui_DATA)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA) ;


