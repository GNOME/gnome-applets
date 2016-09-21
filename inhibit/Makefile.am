NULL =

SUBDIRS = icons src

APPLET_LOCATION=$(pkglibdir)/libinhibit-applet.so

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.InhibitApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

uidir   = $(pkgdatadir)/ui
ui_DATA = inhibit-applet-menu.xml

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA)

EXTRA_DIST = \
	org.gnome.InhibitApplet.panel-applet.in.in \
	$(ui_DATA) \
	$(service_in_files)

-include $(top_srcdir)/git.mk
