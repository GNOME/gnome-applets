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

noinst_PROGRAMS = mc-install-default-macros

EGGFILES = egg-screen-exec.h egg-screen-exec.c

mini_commander_applet_SOURCES =			\
	$(EGGFILES)				\
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

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= mini-commander.schemas

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_MiniCommanderApplet.xml

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;
	$(top_builddir)/mini-commander/src/mc-install-default-macros

@INTLTOOL_SERVER_RULE@

EGGDIR = $(srcdir)/../../../libegg/libegg/screen-exec

regenerate-built-sources:
	EGGFILES="$(EGGFILES)" EGGDIR="$(EGGDIR)" $(srcdir)/update-from-egg.sh

EXTRA_DIST =					\
	update-from-egg.sh			\
	GNOME_MiniCommanderApplet.server.in.in	\
	$(pixmap_DATA)				\
	$(bitmaps_DATA)				\
	$(ui_DATA)				\
	$(schema_DATA)
