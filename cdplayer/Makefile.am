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

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = cdplayer.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@
uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_CDPlayerApplet.xml

IMAGES = images/cdplayer-backward.png images/cdplayer-eject.png images/cdplayer-forward.png images/cdplayer-next.png images/cdplayer-play.png images/cdplayer-pause.png images/cdplayer-prev.png images/cdplayer-prev.png images/cdplayer-stop.png

VARIABLES=cdplayer_stop_data $(srcdir)/images/cdplayer-stop.png \
	  cdplayer_play_data $(srcdir)/images/cdplayer-play.png \
	  cdplayer_pause_data $(srcdir)/images/cdplayer-pause.png \
	  cdplayer_prev_data $(srcdir)/images/cdplayer-prev.png \
	  cdplayer_next_data $(srcdir)/images/cdplayer-next.png \
	  cdplayer_eject_data $(srcdir)/images/cdplayer-eject.png

BUILT_SOURCES = inlinepixbufs.h
CLEANFILES = inlinepixbufs.h

inlinepixbufs.h: $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$(srcdir)/inlinepixbufs.h

$(OBJECTS): inlinepixbufs.h

EXTRA_DIST = \
images/cdplayer-backward.png \
images/cdplayer-eject.png \
images/cdplayer-forward.png \
images/cdplayer-next.png \
images/cdplayer-play.png \
images/cdplayer-pause.png \
images/cdplayer-prev.png \
images/cdplayer-prev.png \
images/cdplayer-stop.png \
cdrom-irix.c \
cdrom-linux.c \
cdrom-solaris.c \
reader-solaris.c \
writer-solaris.c \
$(icon_DATA) \
GNOME_CDPlayerApplet.server.in.in \
$(schemas_in_files)	\
$(ui_DATA) \
$(IMAGES)

install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA);
