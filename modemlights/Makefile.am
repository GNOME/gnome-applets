INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = modemlights_applet

modemlights_applet_SOURCES = \
	modemlights.c	\
	modemlights.h	\
	properties.c

modemlights_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	modemlights_applet.desktop	\
	backgrnd.xpm			\
	backgrnd_s.xpm			\
	button_off.xpm			\
	button_on.xpm			\
	lights.xpm			\
	modemlights_applet.gnorba

sysdir = $(datadir)/applets/Network
sys_DATA = modemlights_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = modemlights_applet.gnorba
