INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(top_builddir)/panel -I$(srcdir)/../../panel	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = multiload_applet

multiload_applet_SOURCES = cpuload.c		\
			   memload.c		\
			   swapload.c		\
			   netload.c		\
			   main.c		\
			   global.h		\
			   properties.c		\
			   properties.h		\
			   property-entries.c	\
			   property-entries.h	\
			   linux-proc.h 	\
			   linux-proc.c 	\
			   load-graph.c		\
			   load-graph.h

multiload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

appletdir	= $(datadir)/applets

Monitorsdir	= $(appletdir)/Monitors

Monitors_DATA	= cpuload_applet.desktop memload_applet.desktop \
		  swapload_applet.desktop netload_applet.desktop

EXTRA_DIST	= $(Monitors_DATA) multiload_applet.gnorba

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = multiload_applet.gnorba 
