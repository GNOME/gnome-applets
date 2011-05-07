SUBDIRS = help

INCLUDES =					\
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


appletdir       = $(datadir)/gnome-panel/4.0/applets
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

schemasdir   = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = drivemount.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

EXTRA_DIST = \
 org.gnome.applets.DriveMountApplet.panel-applet.in.in	\
 $(service_in_files)					\
 $(schemas_in_files)					\
 $(ui_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/drivemount/$$p ; \
		done ; \
	fi
else
install-data-local:
endif

-include $(top_srcdir)/git.mk
