AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(WINDOWTITLE_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

libexec_PROGRAMS = windowtitle

windowtitle_SOURCES = \
	windowtitle.c \
	windowtitle.h \
	preferences.c \
	preferences.h

windowtitle_LDFLAGS = \
	-Wl,--export-dynamic

windowtitle_LDADD = $(WINDOWTITLE_LIBS)

# The new applet files (D-Bus???)
appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.panel.WindowTitleApplet.panel-applet.in
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
service_in_files = org.gnome.panel.applet.WindowTitleAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.WindowTitleAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
	    -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	    $< > $@

# Schemas for GConf
schemasdir       = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = windowtitle.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif

# UI Builder file (Glade)
builderdir = $(datadir)/windowtitle
builder_DATA = windowtitle.ui

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

EXTRA_DIST = \
	org.gnome.panel.WindowTitleApplet.panel-applet.in.in \
	$(service_in_files) \
	$(schemas_in_files) \
	$(builder_DATA)
	

SUBDIRS = \
	pixmaps
