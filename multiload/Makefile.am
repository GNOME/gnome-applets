INCLUDES =						\
	-I$(srcdir)					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(GNOME_APPLETS_CFLAGS)                         \
	$(GTOP_APPLETS_CFLAGS)				\
	-I$(includedir)	

libmultiload_applet_2_la_SOURCES = \
			global.h	\
			linux-proc.h	\
			load-graph.h	\
			local-properties.h	\
			properties.h	\
			cpuload.c	\
			linux-proc.c	\
			load-graph.c	\
			loadavg.c	\
			main.c	\
			memload.c	\
			netload.c	\
			properties.c	\
			swapload.c

libmultiload_applet_2_la_LIBADD = \
				$(GNOME_APPLETS_LIBS) 	\
				$(GTOP_APPLETS_LIBS)

appletdir = $(libdir)
applet_LTLIBRARIES = libmultiload_applet_2.la

GNOME_MultiLoadApplet_Factory.server.in: GNOME_MultiLoadApplet_Factory.server.in.in
	sed -e "s|\@APPLET_LIBDIR\@|$(appletdir)|" $< > $@

pixmaps = gnome-cpu.png gnome-mem.png
pixmapdir = $(datadir)/pixmaps
pixmap_DATA = $(pixmaps)

schemadir   = $(sysconfdir)/gconf/schemas
schema_DATA = multiload.schemas

#install-data-local:
#	GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

EXTRA_DIST =	\
		GNOME_MultiLoadApplet_Factory.server.in.in \
		multiload.schemas \
		$(pixmaps)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MultiLoadApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@