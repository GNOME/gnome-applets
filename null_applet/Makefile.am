INCLUDES = 			\
	-I.			\
	-I$(srcdir)		\
	$(GNOME_APPLETS_CFLAGS)

## The null applet takes over if the mixer isn't built.
if !BUILD_MIXER_APPLET
mixer_server_in = GNOME_MixerApplet.server.in
endif

libexec_PROGRAMS = null_applet

null_applet_SOURCES = null_applet.c

null_applet_LDADD = $(GNOME_APPLETS_LIBS)

serverdir = $(libdir)/bonobo/servers
server_in_files =				\
	GNOME_NullApplet_Factory.server.in	\
	GNOME_CDPlayerApplet.server.in		\
	GNOME_MailcheckApplet_Factory.server.in	\
	GNOME_Panel_WirelessApplet.server.in	\
	GNOME_KeyboardApplet.server.in		\
	$(mixer_server_in)
server_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

GNOME_NullApplet_Factory.server.in: GNOME_NullApplet_Factory.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = GNOME_NullApplet_Factory.server.in $(server_DATA)

## Note that the mixer applet server.in file is always distributed, but
## note always used.
EXTRA_DIST =					\
	GNOME_NullApplet_Factory.server.in.in	\
	GNOME_CDPlayerApplet.server.in		\
	GNOME_MailcheckApplet_Factory.server.in	\
	GNOME_Panel_WirelessApplet.server.in	\
	GNOME_MixerApplet.server.in		\
	GNOME_KeyboardApplet.server.in

-include $(top_srcdir)/git.mk
