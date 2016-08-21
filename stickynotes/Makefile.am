NULL =

SUBDIRS = data help pixmaps src

ui_files = stickynotes-applet-menu.xml \
	stickynotes-note-menu.xml
builder_files = stickynotes-note.ui \
	stickynotes-delete.ui \
	stickynotes-delete-all.ui \
	stickynotes-preferences.ui \
	stickynotes-properties.ui
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

CLEANFILES = \
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
	$(NULL)

-include $(top_srcdir)/git.mk
