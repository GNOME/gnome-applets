SUBDIRS = access help

INCLUDES = \
	-I$(srcdir) \
	-I$(top_srcdir)/gtik/access \
	-I$(top_srcdir)/screen-exec \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"  \
	$(GNOME_APPLETS_CFLAGS)
	$(GNOME_VFS_APPLETS_CFLAGS) \
	-DGTIK_APPLET_NAME=\"gtik2_applet\"

libexec_PROGRAMS = gtik2_applet2

gtik2_applet2_SOURCES = gtik.c 

ACCESS_LT_LIB = $(top_builddir)/gtik/access/libgnomedrawareaaccess.la

gtik2_applet2_LDADD =   $(top_builddir)/screen-exec/libscreen-exec.la \
			$(GNOME_APPLETS_LIBS) \
			$(GNOME_VFS_APPLETS_LIBS) \
			$(ACCESS_LT_LIB) \
			$(GAILUTIL_LIBS)
			
serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GtikApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GtikApplet.xml

@INTLTOOL_SERVER_RULE@

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = gtik.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

CLEANFILES = $(server_DATA) $(server_DATA).in

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
	README AUTHORS ChangeLog \
	GNOME_GtikApplet.server.in.in \
	$(ui_DATA)		\
	$(schemas_in_files)
	
install-data-local:
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
