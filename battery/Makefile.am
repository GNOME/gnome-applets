SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl \
	-I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = battery_applet

battery_applet_SOURCES = battery.c 	\
			 battery.h	\
			 read-battery.c	\
			 read-battery.h	\
			 properties.c	\
			 properties.h	\
			 session.c	\
			 session.h

battery_applet_LDADD = \
			$(GNOME_LIBDIR)  \
			$(GNOME_APPLETS_LIBS) $(INTLLIBS)

sysdir = $(datadir)/applets/Monitors
sys_predata = battery_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-battery.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = battery_applet.gnorba

man_MANS = battery_applet.1

EXTRA_DIST = $(gnorba_DATA) $(sys_DATA) $(sys_predata)	\
	     bolt.xpm bolt-horiz.xpm gnome-battery.png	\
	     $(man_MANS)

