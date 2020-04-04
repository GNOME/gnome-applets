NULL =

noinst_LTLIBRARIES = libmini-commander-applet.la

libmini_commander_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.mini-commander\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libmini_commander_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS)	\
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libmini_commander_applet_la_SOURCES = \
	cmd-completion.c \
	cmd-completion.h\
	command-line.c \
	command-line.h \
	exec.c \
	exec.h \
	mini-commander-applet-private.h \
	mini-commander-applet.c \
	mini-commander-applet.h \
	history.c \
	history.h \
	macro.c \
	macro.h \
	preferences.c \
	preferences.h \
	gsettings.h
	$(NULL)

libmini_commander_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libmini_commander_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(NULL)

BITMAPS = \
	browser-mini.xpm \
	history-mini.xpm \
	$(NULL)

EXTRA_DIST = \
	$(BITMAPS) \
	$(NULL)

-include $(top_srcdir)/git.mk
