# install the upgrading .server file
#
serverdir = $(libdir)/bonobo/servers
server_in_files = \
	GNOME_StickyNotesApplet.server.in
server_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

CLEANFILES = $(server_DATA)
EXTRA_DIST = $(server_in_files)
