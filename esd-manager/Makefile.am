INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(shell esd-config --cflags)

bin_PROGRAMS = esdmanager_applet

esdmanager_applet_SOURCES = \
	esdmanager_applet.c

esdmanager_applet_LDFLAGS = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(ESD_LIBS) $(INTLLIBS)

EXTRA_DIST = esdmanager_applet.desktop esdmanager_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = esdmanager_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = esdmanager_applet.gnorba
