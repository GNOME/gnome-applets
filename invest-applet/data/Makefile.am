SUBDIRS = art

# ******************************************************************************
# Panel Applet stuff
# ******************************************************************************

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.InvestApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

LOCATION=$(pkglibdir)/libinvest-applet.so

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LOCATION\@|$(LOCATION)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

# ******************************************************************************
# Misc data
# ******************************************************************************

uidir = $(pkgdatadir)/ui
ui_DATA = \
       invest-applet-menu.xml

builderdir = $(pkgdatadir)/builder
builder_DATA = \
	prefs-dialog.ui \
	financialchart.ui

investbindir = $(libdir)/invest-applet

# ******************************************************************************
# Build rules
# ******************************************************************************

@INTLTOOL_SCHEMAS_RULE@

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.invest.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

CLEANFILES = $(applet_DATA) $(applet_DATA).in \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

EXTRA_DIST = \
	org.gnome.applets.InvestApplet.panel-applet.in.in \
	$(gsettings_schemas_in_in) \
	$(ui_DATA) \
	$(builder_DATA)

-include $(top_srcdir)/git.mk
