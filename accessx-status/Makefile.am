SUBDIRS = docs pixmaps

libexec_PROGRAMS = accessx-status-applet

accessx_status_applet_SOURCES = \
	applet.c \
	applet.h

accessx_status_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
        $(X_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_AccessxStatusApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_AccessxApplet.xml

INCLUDES   = $(GNOME_APPLETS_CFLAGS)			\
	     -DGLADEDIR=\""$(gladedir)"\"

CLEANFILES = $(server_in_files) $(server_DATA) $(ACCESSX_STATUS_APPLET_CLEANFILES)

EXTRA_DIST = \
	GNOME_AccessxStatusApplet.server.in.in \
	$(ui_DATA)

