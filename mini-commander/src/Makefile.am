NULL =

mini_commander_applet_libdir = $(pkglibdir)
mini_commander_applet_lib_LTLIBRARIES = libmini-commander-applet.la

libmini_commander_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DMC_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.mini-commander\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libmini_commander_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS)	\
	$(GNOME_INCLUDEDIR) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libmini_commander_applet_la_SOURCES = \
	about.c \
	about.h \
	cmd_completion.c \
	cmd_completion.h\
	command_line.c \
	command_line.h \
	exec.c \
	exec.h \
	mini-commander_applet.c \
	mini-commander_applet.h \
	help.c \
	help.h \
	history.c \
	history.h \
	macro.c \
	macro.h \
	preferences.c \
	preferences.h \
	gsettings.h
	$(NULL)

libmini_commander_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libmini_commander_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS)
	$(NULL)

builderdir = $(pkgdatadir)/builder
builder_DATA = mini-commander.ui

BITMAPS = 			\
	browser-mini.xpm	\
	history-mini.xpm

desktop_iconsdir = $(datadir)/icons/hicolor/48x48/apps
desktop_icons_DATA = gnome-mini-commander.png

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor
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

APPLET_LOCATION=$(pkglibdir)/libmini-commander-applet.so

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.MiniCommanderApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache


gsettings_schemas_in_in = \
	org.gnome.gnome-applets.mini-commander.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

uidir = $(pkgdatadir)/ui
ui_DATA = mini-commander-applet-menu.xml

EXTRA_DIST =								\
	$(gsettings_schemas_in_in) \
	org.gnome.applets.MiniCommanderApplet.panel-applet.in.in	\
	$(service_in_files)						\
	$(desktop_icons_DATA)						\
	$(BITMAPS)							\
	$(ui_DATA)							\
	$(builder_DATA)

-include $(top_srcdir)/git.mk
