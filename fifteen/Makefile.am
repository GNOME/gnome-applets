INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = fifteen_applet

fifteen_applet_SOURCES = fifteen.c

fifteen_applet_LDADD = ../../panel/libpanel_applet.la	\
		       $(GNOME_LIBDIR) \
		       $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	fifteen_applet.desktop fifteen_applet.gnorba

sysdir = $(datadir)/applets/Amusements
sys_DATA = fifteen_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = fifteen_applet.gnorba
