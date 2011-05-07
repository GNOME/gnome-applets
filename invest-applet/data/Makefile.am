SUBDIRS = art

# ******************************************************************************
# Panel Applet stuff
# ******************************************************************************
servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.InvestAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.InvestAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.applets.InvestApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache


# ******************************************************************************
# Misc data
# ******************************************************************************
uidir = $(pkgdatadir)/ui
ui_DATA = \
       invest-applet-menu.xml
builder_DATA = \
	prefs-dialog.ui \
	financialchart.ui

investbindir = $(libdir)/invest-applet

# ******************************************************************************
# Build rules
# ******************************************************************************
@INTLTOOL_SERVER_RULE@
@INTLTOOL_SCHEMAS_RULE@



CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA)

DISTCLEANFILES = \
	$(server_in_files) \
	$(server_in_files:.server.in=.server)

EXTRA_DIST = \
	org.gnome.applets.InvestApplet.panel-applet.in.in \
	$(service_in_files) \
	$(ui_DATA) \
	$(builder_DATA)

-include $(top_srcdir)/git.mk
