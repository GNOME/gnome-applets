SUBDIRS = docs
INCLUDES = -I. \
$(GNOME_APPLETS_CFLAGS) \
-I$(srcdir) \
-I$(top_srcdir)/screen-exec \
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

libexec_PROGRAMS = cdplayer_applet2

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
$(CDROM_LIBS) \
$(top_builddir)/screen-exec/libscreen-exec.la

icondir   = $(datadir)/pixmaps
icon_DATA = gnome-cdplayer-icon.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_CDPlayerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = cdplayer.schemas

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_CDPlayerApplet.xml

EXTRA_DIST = \
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
GNOME_CDPlayerApplet.server.in.in \
$(schema_DATA)	\
$(ui_DATA)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;
