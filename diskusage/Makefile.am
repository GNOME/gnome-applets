SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = diskusage_applet

diskusage_applet_SOURCES = \
	diskusage.c		\
	diskusage.h		\
	properties.c		\
	properties.h

diskusage_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST = diskusage_applet.desktop diskusage_applet.gnorba

sysdir = $(datadir)/applets/Monitors
sys_DATA = diskusage_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = diskusage_applet.gnorba
