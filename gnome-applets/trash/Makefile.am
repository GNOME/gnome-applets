NULL =

noinst_LTLIBRARIES = libtrash-applet.la

libtrash_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.trash\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	$(NULL)

libtrash_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(GIO_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtrash_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libtrash_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(GIO_LIBS) \
	$(X_LIBS) \
	$(NULL)

libtrash_applet_la_SOURCES = \
	trash-applet.c \
	trash-applet.h \
	trash-empty.h \
	trash-empty.c \
	$(NULL)

-include $(top_srcdir)/git.mk
