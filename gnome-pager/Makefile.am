INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = gnomepager_applet

gnomepager_applet_SOURCES = \
	gnomepager_applet.c \
	gnomepager.h \
	icon1.xpm \
	icon2.xpm \
	icon3.xpm
	stripe.xbm

gnomepager_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

EXTRA_DIST = gnomepager_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = gnomepager_applet.desktop

