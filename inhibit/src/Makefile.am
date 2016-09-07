NULL =

uidir = $(pkgdatadir)/ui

libexec_PROGRAMS=gnome-inhibit-applet

gnome_inhibit_applet_CFLAGS = \
	-I. \
	-I$(srcdir) \
	-DINHIBIT_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

gnome_inhibit_applet_SOURCES = \
	$(BUILT_SOURCES) \
	inhibit-applet.c \
	$(NULL)

gnome_inhibit_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

gnome_inhibit_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
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

-include $(top_srcdir)/git.mk
