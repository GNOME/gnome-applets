# install the upgrading .server file
#

if BUILD_STICKYNOTES_APPLET
else
serverdir = $(libdir)/bonobo/servers
server_in_files = \
	GNOME_StickyNotesApplet.server.in
server_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

CLEANFILES = $(server_DATA)

endif
EXTRA_DIST = $(server_in_files)
