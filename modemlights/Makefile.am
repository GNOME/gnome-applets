#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = modemlights_applet2

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

@INTLTOOL_SERVER_RULE@

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= modemlights.schemas

EXTRA_DIST = \
	digits.xpm $(server_in_files)	\
	$(schema_DATA) $(ui_DATA)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

