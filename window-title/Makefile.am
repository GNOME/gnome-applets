APPLET_LOCATION = $(pkglibdir)/libwindow-title-applet.so

window_title_applet_libdir = $(pkglibdir)
window_title_applet_lib_LTLIBRARIES = libwindow-title-applet.la

libwindow_title_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DDATA_DIR=\""$(datadir)"\" \
	$(NULL)

libwindow_title_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libwindow_title_applet_la_SOURCES = \
	windowtitle.c \
	windowtitle.h \
	preferences.c \
	preferences.h \
	$(NULL)

libwindow_title_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libwindow_title_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBWNCK_LIBS) \
	$(LIBM) \
	$(NULL)

# The new applet files (D-Bus???)
appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.panel.WindowTitleApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
		-e "s|\@DATADIR\@|$(datadir)|" \
		-e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
	    -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
	    $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.window-title.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN)sed -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

# UI Builder file (Glade)
builderdir = $(pkgdatadir)/builder
builder_DATA = windowtitle.ui

CLEANFILES = $(applet_DATA) $(applet_DATA).in \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

EXTRA_DIST = \
	$(gsettings_schemas_in_in) \
	org.gnome.panel.WindowTitleApplet.panel-applet.in.in \
	$(schemas_in_files) \
	$(builder_DATA)

-include $(top_srcdir)/git.mk
