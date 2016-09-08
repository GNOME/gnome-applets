NULL =

uidir = $(pkgdatadir)/ui

libexec_PROGRAMS = drivemount_applet2

drivemount_applet2_CFLAGS = \
	-I. \
	-I$(srcdir) \
	-DDRIVEMOUNT_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

drivemount_applet2_SOURCES = \
	drivemount.c \
	drive-list.c \
	drive-list.h \
	drive-button.c \
	drive-button.h \
	$(NULL)

drivemount_applet2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

drivemount_applet2_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
