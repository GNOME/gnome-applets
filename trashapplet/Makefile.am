SUBDIRS = src docs

serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_Panel_TrashApplet.server.in
server_DATA = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_Panel_TrashApplet.xml

EXTRA_DIST = \
	$(ui_DATA) \
	GNOME_Panel_TrashApplet.server.in.in

CLEANFILES = $(server_in_files) $(server_DATA)
