INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(top_builddir)/panel \
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = charpick_applet

charpick_applet_SOURCES = charpick.c

charpick_applet_LDADD = ../../panel/libpanel_applet.la	\
		       $(GNOME_LIBDIR) \
		       $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	charpick_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = charpick_applet.desktop charpick_applet.gnorba

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = charpick_applet.gnorba
