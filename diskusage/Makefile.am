INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = diskusage_applet

diskusage_applet_SOURCES = \
	diskusage.c		\
	diskusage.h		\
	diskusage_read.c	\
	properties.c		\
	properties.h

diskusage_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST = diskusage_applet.desktop diskusage_applet.gnorba

sysdir = $(datadir)/applets/Monitors
sys_DATA = diskusage_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = diskusage_applet.gnorba
