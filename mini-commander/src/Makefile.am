## Process this file with automake to produce Makefile.in

INCLUDES = 						\
	 $(GNOME_APPLETS_CFLAGS)			\
	 $(LIBGLADE_CFLAGS)				\
	$(WARN_CFLAGS)					\
	-I.						\
	-I$(srcdir)					\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DMC_GLADEDIR=\""$(gladedir)"\"			\
	$(GNOME_INCLUDEDIR)

libexec_PROGRAMS = mini_commander_applet

tooldir = $(libexecdir)/gnome-applets
tool_PROGRAMS = mc-install-default-macros

mini_commander_applet_SOURCES =			\
	about.c					\
	about.h					\
	cmd_completion.c			\
	cmd_completion.h			\
	command_line.c				\
	command_line.h				\
	exec.c					\
	exec.h					\
	mini-commander_applet.c			\
	mini-commander_applet.h			\
	help.c					\
	help.h					\
	history.c				\
	history.h				\
	macro.c					\
	macro.h					\
	preferences.c				\
	preferences.h				\
	mc-default-macros.h

mini_commander_applet_LDADD =				\
	$(GNOME_APPLETS_LIBS)				\
	$(LIBGLADE_LIBS)

mc_install_default_macros_SOURCES =	\
	mc-install-default-macros.c	\
	mc-default-macros.h

mc_install_default_macros_LDADD =	\
	$(GNOME_APPLETS_LIBS)

gladedir   = $(datadir)/gen_util
glade_DATA = mini-commander.glade

# currently the installation of these bitmaps wouldn't be necessary
bitmapsdir = $(datadir)/pixmaps/mini-commander
bitmaps_DATA = browser-mini.xpm history-mini.xpm

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-mini-commander.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MiniCommanderApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = mini-commander.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_MiniCommanderApplet.xml

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(top_builddir)/mini-commander/src/mc-install-default-macros
endif

@INTLTOOL_SERVER_RULE@

EXTRA_DIST =					\
	GNOME_MiniCommanderApplet.server.in.in	\
	$(pixmap_DATA)				\
	$(bitmaps_DATA)				\
	$(ui_DATA)				\
	$(glade_DATA)				\
	$(schemas_in_files)
