SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = quicklaunch_applet

quicklaunch_applet_SOURCES = quicklaunch.c

quicklaunch_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Utility
sys_DATA = quicklaunch_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = quicklaunch_applet.gnorba

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-quicklaunch.png

EXTRA_DIST = $(sys_DATA) $(gnorba_DATA) $(pixmaps_DATA)
