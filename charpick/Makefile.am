INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = charpick_applet

charpick_applet_SOURCES = charpick.c \
                          charpick.h \
                          properties.c

charpick_applet_LDADD = \
		       $(GNOME_LIBDIR) \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	charpick_applet.desktop	\
	charpick_applet.gnorba \
	charpick.png

charpickbitmapsdir = $(datadir)/pixmaps/
charpickbitmaps_DATA = charpick.png

sysdir = $(datadir)/applets/Utility
sys_DATA = charpick_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = charpick_applet.gnorba
