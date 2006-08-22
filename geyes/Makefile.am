SUBDIRS = themes docs

INCLUDES =						\
	-I.						\
	-I$(srcdir) 					\
	$(GNOME_APPLETS_CFLAGS)				\
	-I$(includedir) 				\
	-DGEYES_THEMES_DIR=\""$(pkgdatadir)/geyes/"\"

libexec_PROGRAMS = geyes_applet2

geyes_applet2_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet2_LDADD = \
	$(GNOME_APPLETS_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GeyesApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GeyesApplet.xml

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = geyes.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

@INTLTOOL_SERVER_RULE@

themesdir = $(pkgdatadir)/geyes

icons =	\
	gnome-eyes-applet.svg	 \
	gnome-eyes-applet.32.png \
	gnome-eyes-applet.24.png \
	gnome-eyes-applet.22.png \
	gnome-eyes-applet.16.png

EXTRA_DIST = \
	     README.themes			\
	     GNOME_GeyesApplet.server.in.in	\
	     $(schemas_in_files)		\
	     $(ui_DATA)				\
	     $(icons)

uninstall-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" == "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		rm -f $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/$$FILE; \
	done

install-data-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" == "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		mkdir -p $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/$$FILE; \
	done
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif
