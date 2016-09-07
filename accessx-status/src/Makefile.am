NULL =

uidir   = $(pkgdatadir)/ui

accessx_status_applet_libdir = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)
accessx_status_applet_lib_LTLIBRARIES = libaccessx-status-applet.la

libaccessx_status_applet_la_CPPFLAGS = \
	-DACCESSX_PIXMAPS_DIR=\""$(pkgdatadir)/accessx-status-applet/pixmaps"\" \
	-DACCESSX_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.accessx-status\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libaccessx_status_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libaccessx_status_applet_la_SOURCES = \
	applet.c \
	applet.h \
	$(NULL)

libaccessx_status_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libaccessx_status_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
