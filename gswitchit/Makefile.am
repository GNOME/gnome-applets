SUBDIRS = help

EXTRA_DIST = \
	$(server_in_in_files) \
        $(ui_DATA) \
	$(schemas_in_files) \
        $(schemas_DATA) \
        $(pixmap_DATA) \
        $(glade_DATA)

libexec_PROGRAMS = gnome-keyboard-applet
noinst_LIBRARIES = libgswitchit-applet.a

gnome_keyboard_applet_SOURCES = switch.c

libgswitchit_applet_a_SOURCES = gswitchit-applet.c \
                         gswitchit-applet.h

if ENABLE_FLAGS
  flags_CFLAGS = -DENABLE_FLAGS=1
endif

COMMON_CFLAGS =				\
	-I$(top_srcdir)			\
	-I.				\
	-DG_LOG_DOMAIN=\"GSwitchIt\"	\
	$(LIBXKLAVIER_CFLAGS)		\
	$(flags_CFLAGS)			\
	$(LIBGLADE_CFLAGS)

COMMON_LIBS = $(LIBXKLAVIER_LIBS) \
	      $(LIBGLADE_LIBS) \
	      $(X_LIBS)

libgswitchit_applet_a_CFLAGS = $(COMMON_CFLAGS) \
           $(GNOME_APPLETS_CFLAGS)

gnome_keyboard_applet_CFLAGS = $(COMMON_CFLAGS) \
           $(GNOME_APPLETS_CFLAGS)

gnome_keyboard_applet_LDADD = \
                         libgswitchit-applet.a \
                         $(top_builddir)/libgswitchit/libgswitchit.a  \
                         $(top_builddir)/libkbdraw/libkbdraw.a \
			 $(COMMON_LIBS) \
                         $(GNOME_APPLETS_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_KeyboardApplet.server.in
server_in_in_files = $(server_in_files:.server.in=.server.in.in) 
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_in_files) 
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

@INTLTOOL_SERVER_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GSwitchItApplet.xml

schemasdir       = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = gswitchit.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
          GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA); \
        fi
endif

pixmapdir = $(datadir)/icons/hicolor/48x48/apps
pixmap_DATA = gswitchit-applet.png gswitchit-properties-capplet.png

glade_DATA = gswitchit-plugins.glade \
             gswitchit.glade

bin_PROGRAMS = gswitchit-plugins-capplet

gswitchit_plugins_capplet_SOURCES = gswitchit-plugins.c \
                                    gswitchit-plugins-add.c \
                                    gswitchit-plugins.h

gswitchit_plugins_capplet_CFLAGS = $(COMMON_CFLAGS) \
            $(GNOME_LIBS2_CFLAGS)

gswitchit_plugins_capplet_LDFLAGS = -export-dynamic

gswitchit_plugins_capplet_LDADD = \
            $(top_builddir)/libgswitchit/libgswitchit.a \
            $(COMMON_LIBS) \
            $(GNOME_LIBS2_LIBS)
 
