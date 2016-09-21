NULL =

tracker_search_bar_libdir=${pkglibdir}
tracker_search_bar_lib_LTLIBRARIES=libtracker-search-bar-applet.la

libtracker_search_bar_applet_la_CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.tracker-search-bar\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-I. \
	-I$(srcdir) \
	$(NULL)

libtracker_search_bar_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(TRACKER_SEARCH_BAR_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtracker_search_bar_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libtracker_search_bar_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(TRACKER_SEARCH_BAR_LIBS) \
	$(NULL)

libtracker_search_bar_applet_la_SOURCES = \
	tracker-applet.c \
	tracker-applet.h \
	tracker-aligned-window.c \
	tracker-aligned-window.h \
	tracker-results-window.c \
	tracker-results-window.h \
	tracker-utils.c \
	tracker-utils.h \
	tracker-resources.c \
	tracker-resources.h \
	$(NULL)

tracker-resources.c: tracker-search-bar-resources.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/tracker-search-bar-resources.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate --c-name tracker $<

tracker-resources.h: tracker-search-bar-resources.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/tracker-search-bar-resources.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate --c-name tracker $<

EXTRA_DIST = \
	tracker-search-bar.ui \
	tracker-search-bar-menu.xml \
	tracker-search-bar-resources.gresource.xml \
	$(NULL)

CLEANFILES = \
	tracker-resources.c \
	tracker-resources.h \
	$(NULL)

-include $(top_srcdir)/git.mk
