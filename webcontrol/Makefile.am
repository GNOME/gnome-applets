INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(top_builddir)/panel \
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = webcontrol_applet

webcontrol_applet_SOURCES = webcontrol.c

webcontrol_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = webcontrol_applet.desktop webcontrol_applet.gnorba

sysdir = $(datadir)/applets/Network
sys_DATA = webcontrol_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = webcontrol_applet.gnorba
