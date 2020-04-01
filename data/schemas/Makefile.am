NULL =

gsettings_schemas_in = \
	org.gnome.gnome-applets.battstat.gschema.xml.in \
	org.gnome.gnome-applets.charpick.gschema.xml.in \
	org.gnome.gnome-applets.command.gschema.xml.in \
	org.gnome.gnome-applets.geyes.gschema.xml.in \
	org.gnome.gnome-applets.gweather.gschema.xml.in \
	org.gnome.gnome-applets.mini-commander.gschema.xml.in \
	org.gnome.gnome-applets.multiload.gschema.xml.in \
	org.gnome.gnome-applets.netspeed.gschema.xml.in \
	org.gnome.gnome-applets.stickynotes.gschema.xml.in \
	org.gnome.gnome-applets.timer.gschema.xml.in \
	org.gnome.gnome-applets.window-buttons.gschema.xml.in \
	org.gnome.gnome-applets.window-picker-applet.gschema.xml.in \
	org.gnome.gnome-applets.window-title.gschema.xml.in \
	$(NULL)

gsettings_ENUM_NAMESPACE = org.gnome.gnome-applets
gsettings_ENUM_FILES = \
	$(NULL)

if BUILD_CPUFREQ_APPLET
gsettings_schemas_in += \
	org.gnome.gnome-applets.cpufreq.gschema.xml.in \
	$(NULL)
gsettings_ENUM_FILES += \
	$(top_srcdir)/gnome-applets/cpufreq/cpufreq-applet.h \
	$(NULL)
endif

gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml: %.gschema.xml.in Makefile
	$(AM_V_GEN) $(SED) \
		-e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' \
		-e "s|\@WB_DATA_DIR\@|$(pkgdatadir)/window-buttons-applet|" \
		< $< > $@

@GSETTINGS_RULES@

EXTRA_DIST = \
	org.gnome.gnome-applets.cpufreq.gschema.xml.in \
	$(gsettings_schemas_in) \
	$(NULL)

CLEANFILES = \
	*.gschema.valid \
	$(gsettings__enum_file) \
	$(gsettings_SCHEMAS) \
	$(NULL)

-include $(top_srcdir)/git.mk
