SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = webcontrol_applet

webcontrol_applet_SOURCES = properties.c webcontrol.c

webcontrol_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = webcontrol_applet.desktop webcontrol_applet.gnorba

sysdir = $(datadir)/applets/Network
sys_DATA = webcontrol_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = webcontrol_applet.gnorba
