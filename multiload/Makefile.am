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

Monitors_predata	= cpuload_applet.desktop.in	\
		memload_applet.desktop.in swapload_applet.desktop.in	\
		netload_applet.desktop.in loadavg_applet.desktop.in
		
Monitors_DATA = $(Monitors_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmaps = gnome-cpu.png gnome-mem.png
pixmapdir = $(datadir)/pixmaps
pixmap_DATA = $(pixmaps)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = multiload_applet.gnorba 

man_MANS = multiload_applet.1

EXTRA_DIST =	\
		$(Monitors_DATA) $(Monitors_predata)	\
		$(gnorba_DATA) $(pixmaps)		\
		$(man_MANS)
		

