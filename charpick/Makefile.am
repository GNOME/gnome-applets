SUBDIRS = help

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"


bin_PROGRAMS = charpick_applet2

charpick_applet2_SOURCES = charpick.c \
                          charpick.h \
                          properties.c

charpick_applet2_LDADD = \
		       $(GNOME_APPLETS_LIBS)

charpickbitmapsdir = $(datadir)/pixmaps/
charpickbitmaps_DATA = charpick.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_CharpickerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= charpick.schemas

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_CharpickerApplet.xml

EXTRA_DIST = \
	charpick.png $(server_in_files)	\
	$(ui_DATA)	\
	$(schema_DATA)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

