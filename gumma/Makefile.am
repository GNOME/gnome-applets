if HAVE_XMMS
xmms = xmms
endif

if APPLET_CDPLAYER
cd = cd
endif

SUBDIRS = gqmpeg $(xmms) $(cd)

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gumma

gumma_LDADD = \
	$(GNOME_LIBDIR)		\
	$(GNOME_APPLETS_LIBS)	\
	$(INTLLIBS)

gumma_SOURCES = \
	gumma.c		\
	gumma.h		\
	led.c		\
	led.h

EXTRA_DIST = \
	gumma.desktop		\
	gumma.gnorba		\
	pixmaps/eject.xpm	\
	pixmaps/led.xpm		\
	pixmaps/next.xpm	\
	pixmaps/play-pause.xpm	\
	pixmaps/prev.xpm	\
	pixmaps/stop.xpm

DIST_SUBDIRS = cd gqmpeg xmms

sysdatadir = $(datadir)/applets/Multimedia
sysdata_DATA = gumma.desktop

gnorbadatadir = $(sysconfdir)/CORBA/servers
gnorbadata_DATA = gumma.gnorba
