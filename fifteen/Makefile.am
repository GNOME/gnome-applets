SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = fifteen_applet

fifteen_applet_SOURCES = fifteen.c

fifteen_applet_LDADD = \
		       $(GNOME_LIBDIR) \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Amusements
sys_predata = fifteen_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-fifteen.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = fifteen_applet.gnorba

man_MANS = fifteen_applet.1

EXTRA_DIST = \
	$(pixmap_DATA)	\
	$(gnorba_DATA)	\
	$(sys_DATA)	\
	$(sys_predata)	\
	$(man_MANS)

