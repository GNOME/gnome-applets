INCLUDES = 			\
	-I.			\
	-I$(srcdir)		\
	$(GNOME_APPLETS_CFLAGS)

libexec_PROGRAMS = null_applet

null_applet_SOURCES = null_applet.c

null_applet_LDADD = $(GNOME_APPLETS_LIBS)

serverdir = $(libdir)/bonobo/servers
server_in_files =				\
	GNOME_NullApplet_Factory.server.in	\
	GNOME_CDPlayerApplet.server.in		\
	GNOME_MailcheckApplet_Factory.server.in	\
	GNOME_Panel_WirelessApplet.server.in
server_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

GNOME_NullApplet_Factory.server.in: GNOME_NullApplet_Factory.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = GNOME_NullApplet_Factory.server.in $(server_DATA)

EXTRA_DIST =					\
	GNOME_NullApplet_Factory.server.in.in	\
	GNOME_CDPlayerApplet.server.in		\
	GNOME_MailcheckApplet_Factory.server.in	\
	GNOME_Panel_WirelessApplet.server.in
