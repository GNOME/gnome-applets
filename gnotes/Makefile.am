INCLUDES = -I. -I$(srcdir)                              \
        -I$(top_srcdir)/intl -I$(top_builddir)/intl     \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gnotes_applet

gnotes_applet_SOURCES = \
        gnotes_applet.c \
	gnotes_applet.h \
        gnote.c \
	gnote.h	\
        gnotes.xpm \
        gnotes_session.c \
	gnotes_session.h \
        properties.c

gnotes_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) \
	$(GNOME_XML_LIB) \
	$(INTLLIBS)

EXTRA_DIST = \
        gnotes_applet.desktop \
	gnotes_applet.h \
	gnotes_session.h \
        gnotes_applet.gnorba \
		gnome-gnotes.png

sysdir = $(datadir)/applets/Utility
sys_DATA = gnotes_applet.desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-gnotes.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gnotes_applet.gnorba

