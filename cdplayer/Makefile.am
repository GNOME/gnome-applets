INCLUDES = -I. \
-I$(srcdir) \
-I$(top_srcdir)/intl \
-I$(top_builddir)/intl	\
-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
-I$(includedir) \
$(GNOME_APPLETS_CFLAGS) \
$(GCONF_APPLETS_CFLAGS) \
$(CDROM_CFLAGS)

bin_PROGRAMS = cdplayer_applet2

cdplayer_applet2_SOURCES =	\
cdplayer.c \
cdplayer.h \
cdrom-interface.h \
led.c \
led.h \
cdrom-$(CDROM_HOST).c 

cdplayer_applet2_LDADD = \
$(GNOME_APPLETS_LIBS) \
$(GCONF_APPLETS_LIBS) \
$(INTLLIBS) \
$(CDROM_LIBS)

icondir   = $(datadir)/pixmaps
icon_DATA = gnome-cdplayer-icon.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_CDPlayerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

EXTRA_DIST = \
images/cdplayer-backward.xpm \
images/cdplayer-eject.xpm \
images/cdplayer-forward.xpm \
images/cdplayer-next.xpm \
images/cdplayer-play-pause.xpm \
images/cdplayer-prev.xpm \
images/cdplayer-prev.xpm \
images/led.xpm \
cdrom-linux.c \
cdrom-solaris.c \
$(icon_DATA) \
$(server_in_files)
