SUBDIRS = help

INCLUDES = 							\
	-I$(top_srcdir)/screen-exec				\
	$(WARN_CFLAGS) 						\
	$(GNOME_APPLETS_CFLAGS) 				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 

mailcheck_applet_SOURCES = \
	mailcheck.c \
	popcheck.c \
	popcheck.h \
	remote-helper.c \
	remote-helper.h

mailcheck_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(top_builddir)/screen-exec/libscreen-exec.la

libexec_PROGRAMS = mailcheck-applet

GNOME_MailcheckApplet_Factory.server.in: GNOME_MailcheckApplet_Factory.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

MAILCHECK_BITMAPS =		\
	email.png		\
	email-e.png		\
	tux-anim.png		\
	email-eclouds.png	\
	email-tiny.png		\
	eclouds.png

mailcheckbitmapsdir   = $(datadir)/pixmaps/mailcheck
mailcheckbitmaps_DATA = $(MAILCHECK_BITMAPS)

soundlistdir       = $(sysconfdir)/sound/events
soundlist_in_files = mailcheck.soundlist.in
soundlist_DATA     = $(soundlist_in_files:.soundlist.in=.soundlist)

@INTLTOOL_SOUNDLIST_RULE@

uidir 	= $(datadir)/gnome-2.0/ui
ui_DATA	= GNOME_MailCheckApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MailcheckApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

schemasdir       = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = mailcheck.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
# don't do this if we are building in eg. rpm
install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA) ;

else
install-data-local:
endif

EXTRA_DIST =						\
	GNOME_MailcheckApplet_Factory.server.in.in	\
	$(MAILCHECK_BITMAPS)				\
	$(soundlist_in_files)				\
	$(schemas_in_files)				\
	$(ui_DATA)

CLEANFILES = $(server_in_files) $(server_DATA)

