EXTRA_DIST = \
	$(server_in_in_files) \
        $(ui_DATA) \
        $(schemas_DATA) \
        $(pixmap_DATA) \
        $(glade_DATA)

libexec_PROGRAMS = gswitchit-applet

gswitchit_applet_SOURCES = gswitchit-applet.c \
                           gswitchit-applet.h

gswitchit_applet_CFLAGS = -I$(top_srcdir) \
           $(LIBXKLAVIER_CFLAGS) \
           $(GNOME_APPLETS_CFLAGS) \
           -DG_LOG_DOMAIN=\"GSwitchIt\" \
           -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
           -I../intl -I$(top_srcdir)/intl

gswitchit_applet_LDADD = $(LIBXKLAVIER_LIBS) \
                         $(GNOME_APPLETS_LIBS) \
                         ../libgswitchit/libgswitchit.a 

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GSwitchItApplet.server.in
server_in_in_files = $(server_in_files:.server.in=.server.in.in) 
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_in_files) 
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@


@INTLTOOL_SERVER_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GSwitchItApplet.xml

schemasdir       = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = gswitchit.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)
                                                                                                      
@INTLTOOL_SCHEMAS_RULE@
                                                                                                      
install-data-local:
	if test -z "$(DESTDIR)" ; then \
          GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA); \
        fi

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gswitchit-applet.png gswitchit-properties-capplet.png

gladedir   =  $(datadir)/gswitchit
glade_DATA = gswitchit-plugins.glade \
             gswitchit-properties.glade
                                                                                          
CAPPLET_CFLAGS = -I$(top_srcdir) -I. -I$(top_srcdir)/intl \
            -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
            -DGLADE_DIR=\""$(gladedir)"\" \
            $(LIBXKLAVIER_CFLAGS) \
            $(LIBGLADE_CFLAGS) \
            $(GNOME_LIBS2_CFLAGS)
                                                                                          
bin_PROGRAMS = gswitchit-plugins-capplet \
               gswitchit-properties-capplet
                                                                                          
gswitchit_plugins_capplet_SOURCES = gswitchit-plugins.c \
                                    gswitchit-plugins-add.c \
                                    gswitchit-plugins.h
                                                                                          
gswitchit_properties_capplet_SOURCES = gswitchit-capplet.c \
                                       gswitchit-capplet.h
                                                                                          
CAPPLET_LDADD = $(LIBXKLAVIER_LIBS) \
            $(LIBGLADE_LIBS) \
            $(GNOME_LIBS2_LIBS) \
            ../libgswitchit/libgswitchit.a
                                                                                          
gswitchit_plugins_capplet_CFLAGS = $(CAPPLET_CFLAGS)
gswitchit_properties_capplet_CFLAGS = $(CAPPLET_CFLAGS)
gswitchit_plugins_capplet_LDADD = $(CAPPLET_LDADD)
gswitchit_properties_capplet_LDADD = $(CAPPLET_LDADD)
