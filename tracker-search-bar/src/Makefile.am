NULL =

tracker_search_bar_CFLAGS = \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-I.					\
	-I$(srcdir)				\
	$(GNOME_APPLETS_CFLAGS)			\
	$(TRACKER_SEARCH_BAR_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libexec_PROGRAMS = tracker-search-bar

tracker_search_bar_SOURCES =                           \
	tracker-applet.c                               \
	tracker-applet.h                               \
	tracker-aligned-window.c                       \
	tracker-aligned-window.h                       \
	tracker-results-window.c                       \
	tracker-results-window.h                       \
	tracker-utils.c                                \
	tracker-utils.h

tracker_search_bar_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

tracker_search_bar_LDADD =                             \
	$(GNOME_APPLETS_LIBS)                          \
	$(TRACKER_SEARCH_BAR_LIBS)

-include $(top_srcdir)/git.mk
