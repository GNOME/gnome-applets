AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(WINDOWBUTTONS_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

libexec_PROGRAMS = windowbuttons

windowbuttons_SOURCES = \
	windowbuttons.c\
	windowbuttons.h \
	preferences.c \
	preferences.h \
	external.c \
	external.h \
	theme.c \
	theme.h

windowbuttons_LDFLAGS = \
	-Wl,--export-dynamic

windowbuttons_LDADD = $(WINDOWBUTTONS_LIBS)

# The new applet files (D-Bus???)
appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.panel.WindowButtonsApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
	    -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	    -e "s|\@DATADIR\@|$(datadir)|" \
	    -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
	    $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

# The new Service file (D-Bus)
servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.WindowButtonsAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.WindowButtonsAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
	    -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	    $< > $@

# Schemas for GConf
schemasdir       = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = windowbuttons.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

windowbuttons.schemas: $(schemas_in_files)
	$(AM_V_GEN)sed \
	    -e "s|\@DATADIR\@|$(datadir)|" \
	    $< > $@

@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif

# UI Builder file (Glade)
builderdir = $(datadir)/windowbuttons
builder_DATA = windowbuttons.ui

#CLEANFILES = $(service_in_files) $(server_DATA)
CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

EXTRA_DIST = \
	org.gnome.panel.WindowButtonsApplet.panel-applet.in.in \
	$(service_in_files) \
	$(schemas_in_files) \
	$(builder_DATA)

SUBDIRS = \
	pixmaps\
	themes
