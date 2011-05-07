## Process this file with automake to produce Makefile.in

INCLUDES = 				\
	-I.				\
	-I$(srcdir)			\
	-DMC_MENU_UI_DIR=\""$(uidir)"\"	\
	$(GNOME_APPLETS_CFLAGS)	\
	$(WARN_CFLAGS)			\
	$(GNOME_INCLUDEDIR)

libexec_PROGRAMS = mini_commander_applet

tooldir = $(libexecdir)/gnome-applets
tool_PROGRAMS = mc-install-default-macros

mini_commander_applet_SOURCES =			\
	about.c					\
	about.h					\
	cmd_completion.c			\
	cmd_completion.h			\
	command_line.c				\
	command_line.h				\
	exec.c					\
	exec.h					\
	mini-commander_applet.c			\
	mini-commander_applet.h			\
	help.c					\
	help.h					\
	history.c				\
	history.h				\
	macro.c					\
	macro.h					\
	preferences.c				\
	preferences.h				\
	mc-default-macros.h

mini_commander_applet_LDADD =	\
	$(GNOME_APPLETS_LIBS)

mc_install_default_macros_SOURCES =	\
	mc-install-default-macros.c	\
	mc-default-macros.h

mc_install_default_macros_LDADD =	\
	$(GNOME_APPLETS_LIBS)

builder_DATA = mini-commander.ui

BITMAPS = 			\
	browser-mini.xpm	\
	history-mini.xpm

desktop_iconsdir = $(iconsdir)/hicolor/48x48/apps
desktop_icons_DATA = gnome-mini-commander.png

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(iconsdir)/hicolor
install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.applets.MiniCommanderApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.MiniCommanderAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.MiniCommanderAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = mini-commander.schemas.in mini-commander-global.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

uidir = $(pkgdatadir)/ui
ui_DATA = mini-commander-applet-menu.xml

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(top_builddir)/mini-commander/src/mc-install-default-macros
endif

EXTRA_DIST =								\
	org.gnome.applets.MiniCommanderApplet.panel-applet.in.in	\
	$(service_in_files)						\
	$(desktop_icons_DATA)						\
	$(BITMAPS)							\
	$(ui_DATA)							\
	$(builder_DATA)							\
	$(schemas_in_files)

-include $(top_srcdir)/git.mk
