INCLUDES = -I. \
$(GNOME_APPLETS_CFLAGS) \
-I$(srcdir) \
-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
-I$(includedir) \
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
$(CDROM_LIBS)

icondir   = $(datadir)/pixmaps
icon_DATA = gnome-cdplayer-icon.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_CDPlayerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = \
images/cdplayer-backward.xpm \
images/cdplayer-eject.xpm \
images/cdplayer-forward.xpm \
images/cdplayer-next.xpm \
images/cdplayer-play-pause.xpm \
images/cdplayer-prev.xpm \
images/cdplayer-prev.xpm \
images/cdplayer-stop.xpm \
images/led.xpm \
cdrom-irix.c \
cdrom-linux.c \
cdrom-solaris.c \
$(icon_DATA) \
$(server_in_files)
