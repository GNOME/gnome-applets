SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = multiload_applet

multiload_applet_SOURCES = cpuload.c		\
			   memload.c		\
			   swapload.c		\
			   netload.c		\
			   loadavg.c		\
			   main.c		\
			   global.h		\
			   properties.c		\
			   properties.h		\
			   local-properties.c	\
			   local-properties.h	\
			   property-entries.c	\
			   property-entries.h	\
			   linux-proc.h 	\
			   linux-proc.c 	\
			   load-graph.c		\
			   load-graph.h

multiload_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

appletdir	= $(datadir)/applets

Monitorsdir	= $(appletdir)/Monitors

Monitors_DATA	= cpuload_applet.desktop memload_applet.desktop \
		  swapload_applet.desktop netload_applet.desktop \
		  loadavg_applet.desktop

pixmaps = gnome-cpu.png gnome-mem.png
pixmapdir = $(datadir)/pixmaps
pixmap_DATA = $(pixmaps)

EXTRA_DIST	= $(Monitors_DATA) multiload_applet.gnorba $(pixmaps)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = multiload_applet.gnorba 
