SUBDIRS = news_scripts docs

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_builddir)	\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = tickastat_applet

module_sources = \
	mod_test.c	\
	mod_coredump.c	\
	mod_loadavg.c	\
	mod_tail.c	\
	mod_news.c

tickastat_applet_SOURCES = \
	main.c		\
	display.c	\
	properties.c	\
	modules.c	\
	tickastat.h	\
	modules.h	\
	$(module_sources)

tickastat_applet_LDADD = \
	$(GNOME_LIBDIR) 	\
	$(GNOME_APPLETS_LIBS) $(INTLLIBS)

sysdir = $(datadir)/applets/Utility
sys_predata = tickastat_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = tickastat_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-ticker.png

man_MANS = tickastat_applet.1

EXTRA_DIST = $(sys_DATA)	\
	$(gnorba_DATA)		\
	$(sys_predata)		\
	noimage.xpm		\
	gnome-ticker.png	\
	$(man_MANS)

