SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/screen-exec			\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DVERSION=\""$(VERSION)"\"

libexec_PROGRAMS = modemlights_applet2

modemlights_applet2_SOURCES = \
	modemlights.c	\
	modemlights.h	\
	properties.c

modemlights_applet2_LDADD = \
		$(top_builddir)/screen-exec/libscreen-exec.la \
		     $(GNOME_APPLETS_LIBS)

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_ModemlightsApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_ModemLightsApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = modemlights.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST =					\
	GNOME_ModemLightsApplet.server.in.in	\
	digits.xpm 				\
	$(schemas_in_files)				\
	$(ui_DATA)

install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA) ;

