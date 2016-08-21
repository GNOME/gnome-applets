NULL =

SUBDIRS = help pixmaps src

APPLET_LOCATION=$(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)/libsticky-notes-applet.so

ui_files = stickynotes-applet-menu.xml \
	stickynotes-note-menu.xml
builder_files = stickynotes-note.ui \
	stickynotes-delete.ui \
	stickynotes-delete-all.ui \
	stickynotes-preferences.ui \
	stickynotes-properties.ui
applet_in_files = org.gnome.applets.StickyNotesApplet.panel-applet.in
gsettings_schemas_in_in = org.gnome.gnome-applets.stickynotes.gschema.xml.in.in

if BUILD_STICKYNOTES_APPLET

builderdir = $(pkgdatadir)/builder
builder_DATA = $(builder_files)

uidir = $(pkgdatadir)/ui
ui_DATA = $(ui_files)

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

appletdir   = $(LIBPANEL_APPLET_DIR)
applet_DATA = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

CLEANFILES = \
	$(applet_DATA) \
	$(applet_DATA).in \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid \
	$(NULL)

endif

EXTRA_DIST = \
	$(gsettings_schemas_in_in) \
	$(builder_files) \
	$(ui_files) \
	$(schemas_in_files) \
	org.gnome.applets.StickyNotesApplet.panel-applet.in.in \
	$(NULL)

-include $(top_srcdir)/git.mk
