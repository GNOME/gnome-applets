NULL =

uidir = $(pkgdatadir)/ui

drivemount_applet_libdir = $(pkglibdir)
drivemount_applet_lib_LTLIBRARIES = libdrive-mount-applet.la

libdrive_mount_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DDRIVEMOUNT_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.drive-mount\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libdrive_mount_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libdrive_mount_applet_la_SOURCES = \
	drivemount.c \
	drive-list.c \
	drive-list.h \
	drive-button.c \
	drive-button.h \
	$(NULL)

libdrive_mount_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libdrive_mount_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
