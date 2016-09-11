NULL =

SUBDIRS = help pixmaps src

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.AccessxStatusApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.AccessxStatusAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.AccessxStatusAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

uidir   = $(pkgdatadir)/ui
ui_DATA = accessx-status-applet-menu.xml

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(ACCESSX_STATUS_APPLET_CLEANFILES)

EXTRA_DIST = \
	org.gnome.applets.AccessxStatusApplet.panel-applet.in.in \
	$(service_in_files) \
	$(ui_DATA)

-include $(top_srcdir)/git.mk
