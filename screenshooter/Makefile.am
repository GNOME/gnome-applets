INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = screenshooter_applet

screenshooter_applet_SOURCES = \
	screenshooter_applet.c		\
	screenshooter_applet.h		\
	root_icon.xpm  \
	window_icon.xpm 

screenshooter_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = screenshooter_applet.desktop screenshooter_applet.gnorba \
	update_scrshot_page 

scrshotdir = $(bindir)
scrshot_SCRIPTS = update_scrshot_page

pngdir = $(datadir)/pixmaps
png_DATA = screenshooter_applet.png

sysdir = $(datadir)/applets/Utility
sys_DATA = screenshooter_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = screenshooter_applet.gnorba
