## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps docs

INCLUDES =					\
	-I.					\
	-I$(srcdir)				\
	$(GNOME_APPLETS_CFLAGS)			\
	$(LIBGLADE_CFLAGS)			\
	$(LIBWNCK_CFLAGS)			\
	-DSTICKYNOTES_GLADEDIR=\"$(gladedir)\"	\
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

stickynotes_applet_LDADD =	\
	$(GNOME_APPLETS_LIBS)	\
	$(LIBGLADE_LIBS)	\
	$(LIBWNCK_LIBS)

icondir = $(datadir)/pixmaps/stickynotes

gladedir = $(datadir)/stickynotes
glade_DATA = stickynotes.glade

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_StickyNotesApplet.xml

schemasdir = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = stickynotes.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_StickyNotesApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = \
	     $(glade_DATA) \
	     $(ui_DATA) \
	     $(schemas_in_files) \
	     GNOME_StickyNotesApplet.server.in.in

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(schemas_DATA)
	$(GCONFTOOL) --shutdown
endif
