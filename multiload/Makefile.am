INCLUDES =						\
	-I$(srcdir)					\
	-I$(top_srcdir)/screen-exec			\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DGNOME_DISABLE_DEPRECATED=1	\
	-DGTK_DISABLE_DEPRECATED=1	\
	$(GNOME_APPLETS_CFLAGS)                         \
	$(GTOP_APPLETS_CFLAGS)				\
	-Wall

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
			swapload.c

multiload_applet_2_LDADD = \
	$(top_builddir)/screen-exec/libscreen-exec.la	\
				$(GNOME_APPLETS_LIBS) 	\
				$(GTOP_APPLETS_LIBS)

pixmaps = gnome-cpu.png

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = $(pixmaps)

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= multiload.schemas

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;
		
EXTRA_DIST =	\
		GNOME_MultiLoadApplet_Factory.server.in \
		GNOME_MultiLoadApplet_Factory.server.in.in \
		multiload.schemas \
		$(pixmaps) \
		$(ui_DATA)

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_MultiloadApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MultiLoadApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@
