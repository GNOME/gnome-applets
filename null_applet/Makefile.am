INCLUDES = -I. -I$(srcdir) \
-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
$(GNOME_APPLETS_CFLAGS)

libexec_PROGRAMS = null_applet

null_applet_SOURCES = null_applet.c

null_applet_LDADD = $(GNOME_APPLETS_LIBS)

serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_NullApplet_Factory.server.in
server_DATA = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

CLEANFILES = $(server_in_files) $(server_DATA)

EXTRA_DIST = GNOME_NullApplet_Factory.server.in.in
