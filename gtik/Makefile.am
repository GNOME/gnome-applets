
INCLUDES =  -I$(srcdir)  -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
		 $(GNOME_APPLETS_CFLAGS) $(GNOME_VFS_APPLETS_CFLAGS) \
		 -DGTIK_APPLET_NAME=\"gtik2_applet\"

bin_PROGRAMS = gtik2_applet2

gtik2_applet2_SOURCES = gtik.c 

gtik2_applet2_LDADD = 	$(GNOME_APPLETS_LIBS) \
			$(GNOME_VFS_APPLETS_LIBS)
			
serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GtikApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GtikApplet.xml

@INTLTOOL_SERVER_RULE@

schemadir 	= @GCONF_SCHEMA_FILE_DIR@
schema_DATA	= gtik.schemas

EXTRA_DIST = \
	README AUTHORS ChangeLog \
	$(server_in_files)	\
	$(ui_DATA)		\
	$(schema_DATA)
	
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ; \
	fi
