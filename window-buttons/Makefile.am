APPLET_LOCATION = $(pkglibdir)/libwindow-buttons-applet.so

window_buttons_applet_libdir = $(pkglibdir)
window_buttons_applet_lib_LTLIBRARIES = libwindow-buttons-applet.la

libwindow_buttons_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DDATA_DIR=\""$(datadir)"\" \
	-DWB_DATA_DIR=\""$(pkgdatadir)/window-buttons-applet"\" \
	$(NULL)

libwindow_buttons_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libwindow_buttons_applet_la_SOURCES = \
	windowbuttons.c \
	windowbuttons.h \
	preferences.c \
	preferences.h \
	external.c \
	external.h \
	theme.c \
	theme.h \
	$(NULL)

libwindow_buttons_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libwindow_buttons_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBWNCK_LIBS) \
	$(LIBM) \
	$(NULL)

# The new applet files (D-Bus???)
appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.panel.WindowButtonsApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
	    -e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
	    -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
	    $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

gsettings_schemas_in_in = org.gnome.gnome-applets.window-buttons.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN)sed \
	    -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' \
	    -e "s|\@PKG_DATADIR\@|$(pkgdatadir)|" \
	    -e "s|\@WB_DATA_DIR\@|$(pkgdatadir)/window-buttons-applet|" \
	    $< > $@

@GSETTINGS_RULES@

# UI Builder file (Glade)
builderdir = $(pkgdatadir)/builder
builder_DATA = windowbuttons.ui

#CLEANFILES = $(service_in_files) $(server_DATA)
CLEANFILES = $(applet_DATA) $(applet_DATA).in \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

EXTRA_DIST = \
	$(gsettings_schemas_in_in) \
	org.gnome.panel.WindowButtonsApplet.panel-applet.in.in \
	$(schemas_in_files) \
	$(builder_DATA)

SUBDIRS = \
	themes

-include $(top_srcdir)/git.mk
