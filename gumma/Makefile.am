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

sysdir = $(datadir)/applets/Multimedia
sys_predata = gumma.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadatadir = $(sysconfdir)/CORBA/servers
gnorbadata_DATA = gumma.gnorba

DIST_SUBDIRS = cd gqmpeg xmms

EXTRA_DIST = \
	pixmaps/eject.xpm	\
	pixmaps/led.xpm		\
	pixmaps/next.xpm	\
	pixmaps/play-pause.xpm	\
	pixmaps/prev.xpm	\
	pixmaps/stop.xpm	\
	$(gnorba_DATA) $(sys_DATA) $(sys_predata)

