## Process this file with automake to produce Makefile.in

DIST_SUBDIRS = src help

if BUILD_MINICOMMANDER_APPLET
SUBDIRS = src help
else
# we need to generate the alternative server file here
serverdir = $(libdir)/bonobo/servers
server_in_files =                               \
	GNOME_MiniCommanderApplet.server.in
server_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

CLEANFILES = $(server_DATA)
EXTRA_DIST = $(server_in_files)
endif
