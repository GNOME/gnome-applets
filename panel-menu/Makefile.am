EXTRA_DIST = panel-menu-icon.png GNOME_PanelMenuApplet.server.in panel-menu.schemas

SUBDIRS = help

icondir  = $(datadir)/pixmaps
icon_DATA = panel-menu-icon.png

INCLUDES = \
	-DPREFIX=\""$(prefix)"\" \
	-DG_LOG_DOMAIN=\"PanelMenu\" \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	-I$(top_srcdir) \
	$(PANEL_MENU_CFLAGS)

lib_LTLIBRARIES = libpanel-menu.la
#bin_PROGRAMS = panel-menu-applet

#panel_menu_applet_SOURCES =
libpanel_menu_la_SOURCES = \
panel-menu-desktop-item.c \
panel-menu-desktop-item.h \
panel-menu-pixbuf.c \
panel-menu-pixbuf.h \
panel-menu-common.c \
panel-menu-common.h \
panel-menu-applications.c \
panel-menu-applications.h \
panel-menu-preferences.c \
panel-menu-preferences.h \
panel-menu-path.c \
panel-menu-path.h \
panel-menu-links.c \
panel-menu-links.h \
panel-menu-directory.c \
panel-menu-directory.h \
panel-menu-documents.c \
panel-menu-documents.h \
panel-menu-actions.c \
panel-menu-actions.h \
panel-menu-windows.c \
panel-menu-windows.h \
panel-menu-workspaces.c \
panel-menu-workspaces.h \
panel-menu-properties.c \
panel-menu-properties.h \
panel-menu-config.c \
panel-menu-config.h \
panel-menu-add.c \
panel-menu-add.h \
panel-menu.c \
panel-menu.h


#panel_menu_applet_LDADD = $(PANEL_MENU_LIBS)
libpanel_menu_la_LIBADD = $(PANEL_MENU_LIBS)

schemadir   = $(sysconfdir)/gconf/schemas
schema_DATA = panel-menu.schemas

serverdir       = $(prefix)/lib/bonobo/servers
server_in_files = GNOME_PanelMenuApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;
