
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

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = \
	README AUTHORS ChangeLog \
	$(server_in_files)
