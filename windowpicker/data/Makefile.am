NULL =

SUBDIRS = \
	theme \
	$(NULL)

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.WindowPicker.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

APPLET_LOCATION=$(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)/libwindow-picker-applet.so

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
	-e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
	-e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
	$< > $@

gsettings_file = org.gnome.gnome-applets.window-picker-applet.gschema.xml
gsettings_SCHEMAS = $(gsettings_file)

@INTLTOOL_XML_NOMERGE_RULE@

$(gsettings_file).in: $(gsettings_file).in.in
	sed \
	  -e "s|\@GETTEXT_PACKAGE\@|$(GETTEXT_PACKAGE)|" \
	  $< > $@

@GSETTINGS_RULES@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

EXTRA_DIST = \
	org.gnome.applets.WindowPicker.panel-applet.in.in \
	$(gsettings_file).in.in

CLEANFILES = $(applet_DATA) \
	$(applet_DATA).in \
	$(gsettings_file).in \
	$(gsettings_file) \
	*.gschema.valid

-include $(top_srcdir)/git.mk

