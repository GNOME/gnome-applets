SUBDIRS = themes docs

INCLUDES =						\
	-I.						\
	-I$(srcdir) 					\
	$(GNOME_APPLETS_CFLAGS)				\
	-I$(includedir) 				\
	-DGEYES_THEMES_DIR=\""$(pkgdatadir)/geyes/"\"	\
	-DGEYES_MENU_UI_DIR=\""$(uidir)"\"

libexec_PROGRAMS = geyes_applet2

geyes_applet2_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet2_LDADD = 		\
	$(GNOME_APPLETS_LIBS)	\
	$(LIBM)

appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.applets.GeyesApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.GeyesAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.GeyesAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

uidir = $(pkgdatadir)/ui
ui_DATA = geyes-applet-menu.xml

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = geyes.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

themesdir = $(pkgdatadir)/geyes

icons =	\
	gnome-eyes-applet.svg	 \
	gnome-eyes-applet.32.png \
	gnome-eyes-applet.24.png \
	gnome-eyes-applet.22.png \
	gnome-eyes-applet.16.png

EXTRA_DIST = \
	     README.themes					\
	     org.gnome.applets.GeyesApplet.panel-applet.in.in 	\
             $(service_in_files)				\
	     $(schemas_in_files)				\
	     $(ui_DATA)						\
	     $(icons)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

uninstall-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" = "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		rm -f $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/$$FILE; \
	done
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated. After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

install-data-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" = "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		mkdir -p $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/$$FILE; \
	done
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated. After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif

-include $(top_srcdir)/git.mk
