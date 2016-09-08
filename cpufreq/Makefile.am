SUBDIRS = help pixmaps src

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.cpufreq.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_ENUM_NAMESPACE = org.gnome.gnome-applets.cpufreq
gsettings_ENUM_FILES = $(top_srcdir)/cpufreq/src/cpufreq-applet.h

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

APPLET_LOCATION = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)/libcpu-frequency-applet.so

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.CPUFreqApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

builderdir = $(pkgdatadir)/builder
builder_DATA = cpufreq-preferences.ui

uidir = $(pkgdatadir)/ui
ui_DATA = cpufreq-applet-menu.xml

CLEANFILES = \
	$(gsettings__enum_file) \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid	\
             $(applet_DATA)			\
             $(applet_DATA).in			\
             $(service_DATA)			\
	     *.bak                              \
	     *.gladep

EXTRA_DIST = \
	$(gsettings_schemas_in_in) \
             org.gnome.applets.CPUFreqApplet.panel-applet.in.in	\
             $(service_in_files)				\
	     $(builder_DATA)                 			\
	     $(ui_DATA)
	     
					


-include $(top_srcdir)/git.mk
