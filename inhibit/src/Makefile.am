NULL =

uidir = $(pkgdatadir)/ui

inhibit_appletlibdir = $(pkglibdir)
inhibit_appletlib_LTLIBRARIES = libinhibit-applet.la

libinhibit_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DINHIBIT_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.inhibit\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libinhibit_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libinhibit_applet_la_SOURCES = \
	$(BUILT_SOURCES) \
	inhibit-applet.c \
	$(NULL)

libinhibit_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libinhibit_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(NULL)

dbus-inhibit.h:
dbus-inhibit.c: org.gnome.SessionManager.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix=org.gnome. \
		--generate-c-code dbus-inhibit \
		--c-namespace DBus \
		$(srcdir)/org.gnome.SessionManager.xml

BUILT_SOURCES = \
	dbus-inhibit.c \
	dbus-inhibit.h \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(NULL)

EXTRA_DIST = \
	org.gnome.SessionManager.xml \
	$(NULL)

-include $(top_srcdir)/git.mk
