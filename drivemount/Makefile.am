SUBDIRS = help

INCLUDES = \
-I. -I$(srcdir) \
-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
$(GNOME_APPLETS_CFLAGS) 

libexec_PROGRAMS = drivemount_applet2

desktop_iconsdir = $(datadir)/icons/hicolor/48x48/apps
desktop_icons_DATA = drivemount-applet.png

drivemount_applet2_SOURCES = \
  drivemount.c \
  drive-list.c \
  drive-list.h \
  drive-button.c \
  drive-button.h

drivemount_applet2_LDADD =				\
	$(GNOME_APPLETS_LIBS) 

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_DriveMountApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_DriveMountApplet.xml

@INTLTOOL_SERVER_RULE@

schemasdir   = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = drivemount.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
GNOME_DriveMountApplet.server.in.in \
$(schemas_in_files)	\
$(ui_DATA)	\
drivemount-applet.png

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/drivemount/$$p ; \
		done ; \
	fi
else
install-data-local:
endif
