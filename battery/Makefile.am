## SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl				\
	-I$(top_builddir)/intl				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(GNOME_APPLETS_CFLAGS)				\
	$(WARN_CFLAGS)

bin_PROGRAMS = battery_applet

battery_applet_SOURCES = battery.c 	\
			 battery.h	\
			 read-battery.c	\
			 read-battery.h	\
			 properties.c	\
			 properties.h	\
			 session.c	\
			 session.h

battery_applet_LDADD = \
			$(GNOME_APPLETS_LIBS) $(INTLLIBS)

serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_BatteryApplet_Factory.server.in
server_DATA = $(server_in_files:.server.in=.server)
## @INTLTOOL_SERVER_RULE@

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-battery.png

EXTRA_DIST = $(server_in_files) \
	     bolt.xpm bolt-horiz.xpm gnome-battery.png

