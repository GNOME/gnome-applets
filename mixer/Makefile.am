INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = mixer_applet

mixer_applet_SOURCES = \
	mixer.c		\
	vslider.c	\
	vslider.h	\
	hslider.c	\
	hslider.h	\
	lamp-small.xpm  \
	lamp-small-red.xpm 

mixer_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = mixer_applet.desktop mixer_applet.gnorba

sysdir = $(datadir)/applets/Multimedia
sys_DATA = mixer_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = mixer_applet.gnorba
