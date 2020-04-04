NULL =

noinst_LTLIBRARIES = libaccessx-status-applet.la

libaccessx_status_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.accessx-status\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libaccessx_status_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(ACCESSX_STATUS_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libaccessx_status_applet_la_SOURCES = \
	accessx-status-applet.c \
	accessx-status-applet.h \
	$(NULL)

libaccessx_status_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libaccessx_status_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(ACCESSX_STATUS_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
