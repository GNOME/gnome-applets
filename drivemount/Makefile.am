SUBDIRS = help

AM_CPPFLAGS =					\
	-I.					\
	-I$(srcdir)				\
	-DDRIVEMOUNT_MENU_UI_DIR=\""$(uidir)"\"	\
	$(GNOME_APPLETS_CFLAGS)			\
	$(GIO_UNIX_CFLAGS)
	$(WARN_CFLAGS)

libexec_PROGRAMS = drivemount_applet2

drivemount_applet2_SOURCES =	\
	drivemount.c		\
	drive-list.c		\
	drive-list.h		\
	drive-button.c		\
	drive-button.h

drivemount_applet2_LDADD =	\
	$(GNOME_APPLETS_LIBS)	\
	$(GIO_UNIX_LIBS)

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.DriveMountApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.DriveMountAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.DriveMountAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

uidir = $(pkgdatadir)/ui
ui_DATA = drivemount-applet-menu.xml

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA)

EXTRA_DIST = \
 org.gnome.applets.DriveMountApplet.panel-applet.in.in	\
 $(service_in_files)					\
 $(ui_DATA)

-include $(top_srcdir)/git.mk
