INCLUDES = \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = whereami_applet

whereami_applet_SOURCES = whereami.c

whereami_applet_LDADD = $(GNOME_LIBDIR) $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = AUTHORS README TODO	\
	whereami_applet.desktop		\
	whereami_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = whereami_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = whereami_applet.gnorba
