## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps docs

INCLUDES = \
	-I. \
	-I$(srcdir) \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBGLADE_CFLAGS) \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPREFIX=\""$(prefix)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DSTICKYNOTES_GLADEDIR=\"$(gladedir)\" \
	-DSTICKYNOTES_ICONDIR=\"$(icondir)\"

libexec_PROGRAMS = stickynotes_applet

stickynotes_applet_SOURCES = \
	util.h \
	util.c \
	stickynotes.h \
	stickynotes_callbacks.h \
	stickynotes_applet.h \
	stickynotes_applet_callbacks.h \
	stickynotes.c \
	stickynotes_callbacks.c \
	stickynotes_applet.c \
	stickynotes_applet_callbacks.c

stickynotes_applet_LDADD = \
	$(GNOME_APPLETS_LIBS)

docdir = ${prefix}/doc/stickynotes
doc_DATA = \
	README \
	TODO \
	ChangeLog

gladedir = $(datadir)/stickynotes
glade_DATA = stickynotes_applet.glade

icondir = $(datadir)/pixmaps/stickynotes

xmldir = $(datadir)/gnome-2.0/ui
xml_in_files = stickynotes_applet.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

schemasdir = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = stickynotes_applet.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

serverdir       = $(libdir)/bonobo/servers
server_in_files = stickynotes_applet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = \
	     $(doc_DATA) \
	     $(glade_DATA) \
	     $(xml_in_files) \
	     $(schemas_in_files) \
	     stickynotes_applet.server.in.in

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(schemas_DATA)
	$(GCONFTOOL) --shutdown

