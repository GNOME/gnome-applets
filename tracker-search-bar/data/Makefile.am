NULL =

appletdir = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.panel.SearchBar.panel-applet.in
applet_DATA = $(applet_in_files:.panel-applet.in=.panel-applet)

APPLET_LOCATION=$(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)/libtracker-search-bar-applet.so

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed                                 \
	  -e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|"         \
	  -e "s|\@VERSION\@|$(PACKAGE_VERSION)|"       \
	  $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

EXTRA_DIST = \
	org.gnome.panel.SearchBar.panel-applet.in.in \
	$(NULL)

CLEANFILES = \
	$(applet_DATA) \
	$(applet_DATA).in \
	$(NULL)

-include $(top_srcdir)/git.mk
