SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = jbc_applet

jbc_applet_SOURCES = \
	jbc-applet.c		\
	jbc-applet.h

jbc_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

SUBDIRS = pixmaps

EXTRA_DIST = jbc_applet.desktop jbc_applet.gnorba README AUTHORS \
             ChangeLog gnome-jbc.png

sysdir = $(datadir)/applets/Clocks
sys_DATA = jbc_applet.desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-jbc.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = jbc_applet.gnorba


# dist-hook:
#	mkdir $(distdir)/pixmaps
#	cp $(srcdir)/pixmaps/*.xpm $(distdir)/pixmaps
