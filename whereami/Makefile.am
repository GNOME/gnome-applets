#SUBDIRS = docs

INCLUDES = $(GNOME_APPLETS_CFLAGS) -DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = whereami_applet2

whereami_applet2_SOURCES = whereami.c

whereami_applet2_LDADD = $(GNOME_APPLETS_LIBS) 

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_WhereamiApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_WheramiApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = AUTHORS README TODO	\
	$(server_in_files) \
	$(ui_DATA)
