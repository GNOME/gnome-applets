INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = jbc_applet

jbc_applet_SOURCES = \
	jbc-applet.c		\
	jbc-applet.h
jbc_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(GNORBA_LIBS)\
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

EXTRA_DIST = jbc_applet.desktop jbc_applet.gnorba README AUTHORS ChangeLog

sysdir = $(datadir)/applets/Utility
sys_DATA = jbc_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = jbc_applet.gnorba


dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*.xpm $(srcdir)/pixmaps
