SUBDIRS = help pixmaps src

schemasdir       = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = cpufreq-applet.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.applets.CPUFreqApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.CPUFreqAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.CPUFreqAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

builder_DATA = cpufreq-preferences.ui

uidir = $(pkgdatadir)/ui
ui_DATA = cpufreq-applet-menu.xml

if GCONF_SCHEMAS_INSTALL
install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/cpufreq/$(schemas_DATA) ;
endif

CLEANFILES = \
             $(applet_DATA)			\
             $(applet_DATA).in			\
             $(service_DATA)			\
	     cpufreq-applet.schemas             \
	     *.bak                              \
	     *.gladep

EXTRA_DIST = \
             org.gnome.applets.CPUFreqApplet.panel-applet.in.in	\
             $(service_in_files)				\
	     $(schemas_in_files)             			\
	     $(builder_DATA)                 			\
	     $(ui_DATA)
	     
					


-include $(top_srcdir)/git.mk
