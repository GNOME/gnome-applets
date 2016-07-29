NULL =

SUBDIRS = src

# Panel applet
appletdir = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.panel.SearchBar.panel-applet.in
applet_DATA = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed                                 \
	  -e "s|\@LIBEXECDIR\@|$(libexecdir)|"         \
	  -e "s|\@VERSION\@|$(PACKAGE_VERSION)|"       \
	  $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

# Applet UIs and menu
uidir = $(pkgdatadir)/ui
ui_DATA =                                             \
	tracker-search-bar.ui                         \
	tracker-search-bar-menu.xml

# DBus service file
servicedir = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.SearchBarFactory.service.in
service_DATA = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.SearchBarFactory.service: $(service_in_files)
	$(AM_V_GEN)sed                                \
	  -e "s|\@LIBEXECDIR\@|$(libexecdir)|"        \
	  $< > $@

# Distribution details
EXTRA_DIST = \
	org.gnome.panel.SearchBar.panel-applet.in.in \
	$(ui_DATA) \
	$(service_in_files)

CLEANFILES = \
	$(applet_DATA) \
	$(applet_DATA).in \
	$(service_DATA)

-include $(top_srcdir)/git.mk
