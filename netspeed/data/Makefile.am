appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.panel.Netspeed.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
	-e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	-e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
	$< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.NetspeedAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.NetspeedAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
	-e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	$< > $@

uidir = $(pkgdatadir)/ui
ui_DATA = netspeed-menu.xml

gsettings_schemas_in_in = org.gnome.gnome-applets.netspeed.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

EXTRA_DIST = \
	org.gnome.panel.Netspeed.panel-applet.in.in \
	$(gsettings_schemas_in_in) \
	$(service_in_files) \
	$(ui_DATA)

CLEANFILES = \
	$(applet_DATA) \
	$(applet_DATA).in \
	$(service_DATA) \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

-include $(top_srcdir)/git.mk

