NULL =

SUBDIRS = help src

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.CharpickerApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.CharpickerAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.CharpickerAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) \
	$(gsettings_SCHEMAS_in)	\
	$(gsettings_SCHEMAS)	\
	*.gschema.valid	

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.charpick.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

uidir = $(pkgdatadir)/ui
ui_DATA = charpick-applet-menu.xml

EXTRA_DIST =							\
	$(gsettings_schemas_in_in) \
	org.gnome.applets.CharpickerApplet.panel-applet.in.in	\
	$(service_in_files)					\
	$(ui_DATA)

-include $(top_srcdir)/git.mk
