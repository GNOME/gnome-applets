SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = another_clock_applet

another_clock_applet_SOURCES = 			\
				another_clock.c	


another_clock_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)

EXTRA_DIST = 					\
	    another_clock.xpm			\
	    another_clock_applet.desktop	\
	    another_clock_applet.gnorba \
		gnome-another-clock.png


another_clockbitmapsdir = $(datadir)/pixmaps
another_clockbitmaps_DATA = another_clock.xpm

sysdir = $(datadir)/applets/Clocks
sys_DATA = another_clock_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = another_clock_applet.gnorba

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-another-clock.png
