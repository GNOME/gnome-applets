INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
       -I$(srcdir)/../../panel                         \
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = multiload_applet

multiload_applet_SOURCES = cpuload.c 	\
			   cpuload.h	\
			   memload.c	\
			   memload.h	\
			   swapload.c	\
			   swapload.h	\
			   main.c		\
			   linux-proc.h 	\
			   linux-proc.c 	\
			   properties-cpu.c	\
			   properties-cpu.h	\
			   properties-mem.c	\
			   properties-mem.h	\
			   properties-swap.c	\
			   properties-swap.h

multiload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST	= cpuload_applet.desktop memload_applet.desktop \
		  swapload_applet.desktop multiload_applet.gnorba

appletdir	= $(datadir)/applets

Monitorsdir	= $(appletdir)/Monitors

Monitors_DATA	= cpuload_applet.desktop memload_applet.desktop \
		  swapload_applet.desktop

EXTRA_DIST	= $(Monitors_DATA)

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = multiload_applet.gnorba 
