SUBDIRS = docs

pixmapsdir = $(datadir)/pixmaps/mixer
pixmaps_DATA =  gnome-mixer-applet.png		\
 		volume-max.png			\
 		volume-medium.png		\
 		volume-min.png			\
 		volume-mute.png			\
 		volume-zero.png

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/screen-exec			\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED \
	$(GNOME_APPLETS_CFLAGS)				

libexec_PROGRAMS = mixer_applet2

mixer_applet2_SOURCES = 				\
	mixer.c

mixer_applet2_LDADD = 					\
	$(MIXER_LIBS) 					\
	$(GNOME_APPLETS_LIBS)				\
	$(top_builddir)/screen-exec/libscreen-exec.la

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MixerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

uidir   = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_MixerApplet.xml

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = mixer.schemas

EXTRA_DIST = 						\
	GNOME_MixerApplet.server.in.in			\
	$(ui_DATA) 					\
	$(pixmaps_DATA)					\
	$(schema_DATA)
	
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;
