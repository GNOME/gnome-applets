INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-I$(LIBGTOP_INCS)

bin_PROGRAMS = cpuload_applet

cpuload_applet_SOURCES = cpuload.c 	\
			 linux-proc.h 	\
			 linux-proc.c 	\
			 properties.c	\
			 properties.h

cpuload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST = cpuload_applet.desktop cpuload_applet.gnorba

sysdir = $(datadir)/applets/Monitors
sys_DATA = cpuload_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = cpuload_applet.gnorba 
