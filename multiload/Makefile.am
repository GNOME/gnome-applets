SUBDIRS = docs

INCLUDES =			\
	-I$(srcdir)		\
	$(GNOME_APPLETS_CFLAGS)	\
	$(GTOP_APPLETS_CFLAGS)

libexec_PROGRAMS = multiload-applet-2

multiload_applet_2_SOURCES = \
			global.h	\
			linux-proc.h	\
			load-graph.h	\
			cpuload.c	\
			linux-proc.c	\
			load-graph.c	\
			loadavg.c	\
			main.c	\
			memload.c	\
			netload.c	\
			properties.c	\
			swapload.c	\
			diskload.c

multiload_applet_2_LDADD = \
				$(GNOME_APPLETS_LIBS) 	\
				$(GTOP_APPLETS_LIBS)

desktop_iconsdir = $(iconsdir)/hicolor/48x48/apps
desktop_icons_DATA = gnome-cpu.png

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = multiload.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST =						\
	GNOME_MultiLoadApplet_Factory.server.in.in	\
	$(schemas_in_files)				\
	$(desktop_icons_DATA)				\
	$(ui_DATA)

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_MultiloadApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MultiLoadApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

@INTLTOOL_SERVER_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif
