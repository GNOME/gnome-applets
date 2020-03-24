NULL =

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.battstat.gschema.xml.in.in \
	org.gnome.gnome-applets.charpick.gschema.xml.in.in \
	org.gnome.gnome-applets.command.gschema.xml.in.in \
	org.gnome.gnome-applets.geyes.gschema.xml.in.in \
	org.gnome.gnome-applets.gweather.gschema.xml.in.in \
	org.gnome.gnome-applets.mini-commander.gschema.xml.in.in \
	org.gnome.gnome-applets.multiload.gschema.xml.in.in \
	org.gnome.gnome-applets.netspeed.gschema.xml.in.in \
	org.gnome.gnome-applets.stickynotes.gschema.xml.in.in \
	org.gnome.gnome-applets.timer.gschema.xml.in.in \
	org.gnome.gnome-applets.window-buttons.gschema.xml.in.in \
	org.gnome.gnome-applets.window-picker-applet.gschema.xml.in.in \
	org.gnome.gnome-applets.window-title.gschema.xml.in.in \
	$(NULL)

gsettings_ENUM_NAMESPACE = org.gnome.gnome-applets
gsettings_ENUM_FILES = \
	$(NULL)

if BUILD_CPUFREQ_APPLET
gsettings_schemas_in_in += \
	org.gnome.gnome-applets.cpufreq.gschema.xml.in.in \
	$(NULL)
gsettings_ENUM_FILES += \
	$(top_srcdir)/cpufreq/src/cpufreq-applet.h \
	$(NULL)
endif

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) \
		-e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' \
		-e "s|\@PKG_DATADIR\@|$(pkgdatadir)|" \
		-e "s|\@WB_DATA_DIR\@|$(pkgdatadir)/window-buttons-applet|" \
		< $< > $@

@GSETTINGS_RULES@

EXTRA_DIST = \
	org.gnome.gnome-applets.cpufreq.gschema.xml.in.in \
	$(gsettings_schemas_in_in) \
	$(NULL)

CLEANFILES = \
	*.gschema.valid \
	$(gsettings__enum_file) \
	$(gsettings_schemas_in) \
	$(gsettings_SCHEMAS) \
	$(NULL)

-include $(top_srcdir)/git.mk
