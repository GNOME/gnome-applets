#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DVERSION=\""$(VERSION)"\"

libexec_PROGRAMS = modemlights_applet2

modemlights_applet2_SOURCES = \
	modemlights.c	\
	modemlights.h	\
	properties.c

modemlights_applet2_LDADD = \
		     $(GNOME_APPLETS_LIBS)

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_ModemlightsApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_ModemLightsApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= modemlights.schemas

EXTRA_DIST =					\
	GNOME_ModemLightsApplet.server.in.in	\
	digits.xpm 				\
	$(schema_DATA)				\
	$(ui_DATA)

install-data-local:
	for p in $(schema_DATA); do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$$p ; \
	done

