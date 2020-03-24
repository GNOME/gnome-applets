NULL =

APPLET_LOCATION=$(pkglibdir)/libnet-speed-applet.so

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.panel.Netspeed.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
	-e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
	-e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
	$< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

uidir = $(pkgdatadir)/ui
ui_DATA = netspeed-menu.xml

EXTRA_DIST = \
	org.gnome.panel.Netspeed.panel-applet.in.in \
	$(ui_DATA) \
	$(NULL)

CLEANFILES = \
	$(applet_DATA) \
	$(applet_DATA).in \
	$(NULL)

-include $(top_srcdir)/git.mk
