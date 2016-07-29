NULL =

tracker_search_bar_libdir=${pkglibdir}/${LIBPANEL_APPLET_API_VERSION}
tracker_search_bar_lib_LTLIBRARIES=libtracker-search-bar-applet.la

libtracker_search_bar_applet_la_CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-I. \
	-I$(srcdir) \
	$(NULL)

libtracker_search_bar_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(TRACKER_SEARCH_BAR_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtracker_search_bar_appet_la_LDFLAGS = \
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
	$(NULL)

-include $(top_srcdir)/git.mk
