NULL =

SUBDIRS = data src

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.timer.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

EXTRA_DIST =							\
	$(gsettings_schemas_in_in) \
	$(NULL)

CLEANFILES = \
	$(gsettings_SCHEMAS) \
	*.gschema.valid \
	$(NULL)

-include $(top_srcdir)/git.mk
