INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = hal_applet 

hal_applet_SOURCES = hal.c

hal_applet_LDADD = ../../panel/libpanel_applet.la	\
		       $(GNOME_LIBDIR) \
		       $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	hal_applet.desktop hal_applet.gnorba hal.png

halbitmapsdir = $(datadir)/pixmaps/hal
halbitmaps_DATA = hal.png

sysdir = $(datadir)/applets/Amusements
sys_DATA = hal_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = hal_applet.gnorba
