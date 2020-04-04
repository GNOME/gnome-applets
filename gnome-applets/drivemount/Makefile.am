NULL =

noinst_LTLIBRARIES = libdrive-mount-applet.la

libdrive_mount_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.drive-mount\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libdrive_mount_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libdrive_mount_applet_la_SOURCES = \
	drivemount-applet.c \
	drivemount-applet.h \
	drive-list.c \
	drive-list.h \
	drive-button.c \
	drive-button.h \
	$(NULL)

libdrive_mount_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libdrive_mount_applet_la_LIBADD = \
	$(GNOME_PAENL_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
