NULL =

noinst_LTLIBRARIES = libsticky-notes-applet.la

libsticky_notes_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.sticky-notes\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libsticky_notes_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(XML2_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libsticky_notes_applet_la_SOURCES = \
	gsettings.h \
	sticky-notes.c \
	sticky-notes.h \
	sticky-notes-applet.c \
	sticky-notes-applet.h \
	sticky-notes-preferences.c \
	sticky-notes-preferences.h \
	util.c \
	util.h \
	$(NULL)

libsticky_notes_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libsticky_notes_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(LIBWNCK_LIBS) \
	$(X_LIBS) \
	$(XML2_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
