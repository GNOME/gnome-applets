NULL =

uidir   = $(pkgdatadir)/ui

libexec_PROGRAMS = accessx-status-applet

accessx_status_applet_CFLAGS = \
	-DACCESSX_PIXMAPS_DIR=\""$(pkgdatadir)/accessx-status-applet/pixmaps"\" \
	-DACCESSX_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

accessx_status_applet_SOURCES = \
	applet.c \
	applet.h \
	$(NULL)

accessx_status_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

accessx_status_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
