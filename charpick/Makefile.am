SUBDIRS = help

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/screen-exec			\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"


libexec_PROGRAMS = charpick_applet2

charpick_applet2_SOURCES = charpick.c \
                          charpick.h \
                          properties.c

charpick_applet2_LDADD = \
	$(top_builddir)/screen-exec/libscreen-exec.la \
		       $(GNOME_APPLETS_LIBS)

charpickbitmapsdir = $(datadir)/pixmaps/
charpickbitmaps_DATA = charpick.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_CharpickerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

@INTLTOOL_SERVER_RULE@

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = charpick.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@
uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_CharpickerApplet.xml

EXTRA_DIST =					\
	charpick.png				\
	GNOME_CharpickerApplet.server.in.in	\
	$(ui_DATA)				\
	$(schemas_in_files)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;

