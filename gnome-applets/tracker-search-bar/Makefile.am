NULL =

if HAVE_TRACKER_SEARCH_BAR
noinst_LTLIBRARIES=libtracker-search-bar-applet.la

libtracker_search_bar_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.tracker-search-bar\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libtracker_search_bar_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(TRACKER_SEARCH_BAR_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtracker_search_bar_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libtracker_search_bar_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(TRACKER_SEARCH_BAR_LIBS) \
	$(NULL)

libtracker_search_bar_applet_la_SOURCES = \
	tracker-applet.c \
	tracker-applet.h \
	tracker-aligned-window.c \
	tracker-aligned-window.h \
	tracker-results-window.c \
	tracker-results-window.h \
	tracker-regex.c \
	tracker-regex.h \
	$(NULL)
endif

-include $(top_srcdir)/git.mk
