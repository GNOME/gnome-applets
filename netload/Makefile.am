INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	$(LIBGTOP_INCS)

bin_PROGRAMS = netload_applet

netload_applet_SOURCES = \
	netload.c 	\
	netload.h	\
	linux-proc.h 	\
	linux-proc.c 	\
	properties.c	\
	properties.h

netload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST = netload_applet.desktop netload_applet.gnorba

sysdir = $(datadir)/applets/Network
sys_DATA = netload_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = netload_applet.gnorba 
