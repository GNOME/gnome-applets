SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl \
	-I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

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
			$(GNOME_LIBDIR)  \
			$(GNOME_APPLETS_LIBS) $(INTLLIBS)

EXTRA_DIST = battery_applet.desktop battery_applet.gnorba \
	     bolt.xpm bolt-horiz.xpm gnome-battery.png

sysdir = $(datadir)/applets/Monitors
sys_DATA = battery_applet.desktop

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-battery.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = battery_applet.gnorba
