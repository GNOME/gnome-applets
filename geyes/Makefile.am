#SUBDIRS = themes docs
SUBDIRS = themes

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 				\
	-DGEYES_THEMES_DIR=\""$(datadir)/geyes/"\"

bin_PROGRAMS = geyes_applet2

geyes_applet2_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet2_LDADD = \
	$(GNOME_APPLETS_LIBS)


serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GeyesApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GeyesApplet.xml

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= geyes.schemas

@INTLTOOL_SERVER_RULE@

themesdir = $(datadir)/geyes

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-eyes.png

EXTRA_DIST = \
	     geyes.h \
	     gnome-eyes.png \
	     README.themes \
	     $(pixmap_DATA) \
	     $(server_in_files) \
	     $(schema_DATA) \
	     $(ui_DATA)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;


