SUBDIRS = docs
INCLUDES = -I. \
$(GNOME_APPLETS_CFLAGS) \
-I$(srcdir) \
-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"  \
$(CDROM_CFLAGS)

if CDDA_SOLARIS
CDROM_CDDA_SRCS = \
	reader-solaris.c \
	writer-solaris.c 
	
CDROM_CDDA_DEPS = \
	reader-solaris.c \
	writer-solaris.c
endif

bin_PROGRAMS = cdplayer_applet2

EGGFILES = egg-screen-exec.h egg-screen-exec.c

cdplayer_applet2_SOURCES = \
cdplayer.c \
cdplayer.h \
cdrom-interface.h \
led.c \
led.h \
$(EGGFILES) \
reader-solaris.h \
writer-solaris.h \
cdda-solaris.h \
cdrom-@CDROM_HOST@.c \
$(CDROM_CDDA_SRCS)

cdplayer_applet2_DEPENDS = \
cdrom-@CDROM_HOST@.c \
$(CDROM_CDDA_DEPS)

cdplayer_applet2_LDADD = \
$(GNOME_APPLETS_LIBS) \
$(CDROM_LIBS)

icondir   = $(datadir)/pixmaps
icon_DATA = gnome-cdplayer-icon.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_CDPlayerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = cdplayer.schemas

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_CDPlayerApplet.xml

EXTRA_DIST = \
update-from-egg.sh \
images/cdplayer-backward.xpm \
images/cdplayer-eject.xpm \
images/cdplayer-forward.xpm \
images/cdplayer-next.xpm \
images/cdplayer-play-pause.xpm \
images/cdplayer-prev.xpm \
images/cdplayer-prev.xpm \
images/cdplayer-stop.xpm \
cdrom-irix.c \
cdrom-linux.c \
cdrom-solaris.c \
reader-solaris.c \
writer-solaris.c \
$(icon_DATA) \
$(server_in_files) \
$(schema_DATA)	\
$(ui_DATA)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;

EGGDIR = $(srcdir)/../../libegg/libegg/screen-exec

regenerate-built-sources:
	EGGFILES="$(EGGFILES)" EGGDIR="$(EGGDIR)" $(srcdir)/update-from-egg.sh
