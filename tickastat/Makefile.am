SUBDIRS = docs

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
sys_DATA = tickastat_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = tickastat_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-ticker.png

EXTRA_DIST = $(sys_DATA) $(gnorba_DATA) noimage.xpm gnome-ticker.png

SUBDIRS = news_scripts

