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
sys_predata = quicklaunch_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = quicklaunch_applet.gnorba

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-quicklaunch.png

man_MANS = quicklaunch_applet.1

EXTRA_DIST = $(pixmaps_DATA)	\
	$(gnorba_DATA)		\
	$(sys_DATA)		\
	$(sys_predata)		\
	$(man_MANS)
	

