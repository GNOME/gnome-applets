INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = dialer_applet

dialer_applet_SOURCES =	\
        dialer.c	\
	dialer.h

dialer_applet_LDADD = ../../panel/libpanel_applet.la		\
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS)

EXTRA_DIST = dialer_applet.desktop dialer_applet.gnorba

sysdir = $(datadir)/applets/Network
sys_DATA = dialer_applet.desktop

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = dialer_applet.gnorba
