EXTRA_DIST = \
	$(server_in_in_files) \
        $(ui_DATA) \
        $(schemas_DATA) \
        $(pixmap_DATA) \
        $(glade_DATA)

libexec_PROGRAMS = gswitchit-applet

gswitchit_applet_SOURCES = switch.c \
                           gswitchit-applet.c \
                           gswitchit-capplet.c \
                           gswitchit-applet.h

COMMON_CFLAGS = -I$(top_srcdir) -I. -I$(top_srcdir)/intl \
           -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
           -DG_LOG_DOMAIN=\"GSwitchIt\" \
           -DGLADE_DIR=\""$(gladedir)"\" \
           $(LIBXKLAVIER_CFLAGS) \
           $(LIBGLADE_CFLAGS)

if ENABLE_FLAGS
  COMMON_CFLAGS += -DENABLE_FLAGS=1
endif

COMMON_LIBS = $(LIBXKLAVIER_LIBS) \
              $(LIBGLADE_LIBS) \
              ../libgswitchit/libgswitchit.a 

gswitchit_applet_CFLAGS = $(COMMON_CFLAGS) \
           $(GNOME_APPLETS_CFLAGS)

gswitchit_applet_LDADD = $(COMMON_LIBS) \
                         $(GNOME_APPLETS_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_KeyboardApplet.server.in
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
                                                                                          
bin_PROGRAMS = gswitchit-plugins-capplet
                                                                                          
gswitchit_plugins_capplet_SOURCES = gswitchit-plugins.c \
                                    gswitchit-plugins-add.c \
                                    gswitchit-plugins.h
                                                                                          
gswitchit_plugins_capplet_CFLAGS = $(COMMON_CFLAGS) \
            $(GNOME_LIBS2_CFLAGS)
                                                                                          
gswitchit_plugins_capplet_LDADD = $(COMMON_LIBS) \
            $(GNOME_LIBS2_LIBS)
                                                                        
