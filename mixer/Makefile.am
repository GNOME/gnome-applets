#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(GNOME_APPLETS_CFLAGS)				\
	-I$(includedir)

bin_PROGRAMS = mixer_applet2

mixer_applet2_SOURCES = 				\
	mixer.c

mixer_applet2_LDADD = 					\
	$(MIXER_LIBS) 					\
	$(GNOME_APPLETS_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MixerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

EXTRA_DIST = 						\
	$(server_DATA) 					\
	$(server_in_files)

