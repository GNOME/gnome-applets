NULL =

noinst_LTLIBRARIES = libinhibit-applet.la

libinhibit_applet_la_CPPFLAGS = \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.inhibit\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libinhibit_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libinhibit_applet_la_SOURCES = \
	inhibit-applet.c \
	inhibit-applet.h \
	$(BUILT_SOURCES) \
	$(NULL)

libinhibit_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libinhibit_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
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
