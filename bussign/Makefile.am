INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(top_builddir)/panel \
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = bussign_applet

bussign_applet_SOURCES = \
	bussign.c	\
	bussign.h


bussign_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(GNOMEGNORBA_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)      \
                     $(GHTTP_LIB)

EXTRA_DIST = bussign_applet.desktop

sysdir = $(datadir)/applets/Amusements
sys_DATA = bussign_applet.desktop
