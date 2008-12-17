## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps docs data

ui_files = GNOME_StickyNotesApplet.xml
builder_files = stickynotes.ui
server_in_files = GNOME_StickyNotesApplet.server.in
schemas_in_files = stickynotes.schemas.in

if BUILD_STICKYNOTES_APPLET
INCLUDES =					\
	-I.					\
	-I$(srcdir)				\
	$(GNOME_APPLETS_CFLAGS)			\
	$(GNOME_LIBS2_CFLAGS)			\
	$(LIBWNCK_CFLAGS)			\
	$(XML2_CFLAGS)				\
	-DSTICKYNOTES_ICONDIR=\"$(datadir)/pixmaps/stickynotes/\"

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
	$(LIBWNCK_LIBS)		\
	$(GNOME_LIBS2_LIBS)	\
	$(XML2_LIBS)

builder_DATA = $(builder_files)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = $(ui_files)

schemasdir = @GCONF_SCHEMA_FILE_DIR@
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

serverdir       = $(libdir)/bonobo/servers
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

@INTLTOOL_SERVER_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(schemas_DATA)
endif
endif

EXTRA_DIST = \
	     $(builder_files) \
	     $(ui_files) \
	     $(schemas_in_files) \
	     GNOME_StickyNotesApplet.server.in.in

