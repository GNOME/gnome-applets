SUBDIRS = icons

AM_CPPFLAGS =					\
	-I.					\
	-I$(srcdir)				\
	-DINHIBIT_MENU_UI_DIR=\""$(uidir)"\"	\
	$(GNOME_APPLETS_CFLAGS)			\
	$(WARN_CFLAGS)				\
	$(NULL)

libexec_PROGRAMS=gnome-inhibit-applet

gnome_inhibit_applet_SOURCES =					\
	dbus-inhibit.h					\
	dbus-inhibit.c					\
	inhibit-applet.c

gnome_inhibit_applet_LDADD =					\
	$(GNOME_APPLETS_LIBS)

dbus-inhibit.h:
dbus-inhibit.c: org.gnome.SessionManager.xml Makefile.am
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix=org.gnome. \
		--generate-c-code dbus-inhibit \
		--c-namespace DBus \
		org.gnome.SessionManager.xml

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.InhibitApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.InhibitAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.InhibitAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

uidir   = $(pkgdatadir)/ui
ui_DATA = inhibit-applet-menu.xml

BUILT_SOURCES = \
	dbus-inhibit.c \
	dbus-inhibit.h \
	$(NULL)

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(BUILT_SOURCES) $(service_DATA)

EXTRA_DIST = \
	org.gnome.InhibitApplet.panel-applet.in.in \
	$(ui_DATA) \
	$(service_in_files)

-include $(top_srcdir)/git.mk
