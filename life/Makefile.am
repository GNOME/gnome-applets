SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 

bin_PROGRAMS = life_applet

life_applet_SOURCES = life.c

life_applet_LDADD = \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Amusements
sys_predata = life_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = life_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-life.png

EXTRA_DIST = $(pixmap_DATA)	\
	$(gnorba_DATA) $(sys_DATA) $(sys_predata)
