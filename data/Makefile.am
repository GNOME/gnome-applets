serverdir   = $(libdir)/bonobo/servers
server_in_files = GNOME_WindowPicker.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

$(server_in_files): $(server_in_files).in Makefile
	sed -e "s|\@LOCATION\@|$(APPLET_LOCATION)|" \
	$< > $@

APPLET_LOCATION=$(libexecdir)/window-picker-applet

schemasdir = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = window-picker-applet.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = $(server_DATA).in.in $(schemas_in_files)
CLEANFILES = $(server_DATA) $(server_DATA).in $(schemas_DATA)

clean:
	rm -f $(desktop_DATA) $(desktop_DATA).in $(server_DATA) $(server_DATA).in	
