SUBDIRS = docs

INCLUDES =			\
	-I$(srcdir)		\
	-DMULTILOAD_MENU_UI_DIR=\""$(uidir)"\" \
	$(GNOME_APPLETS_CFLAGS)	\
	$(GTOP_APPLETS_CFLAGS)	\
	$(GIO_UNIX_CFLAGS)

libexec_PROGRAMS = multiload-applet-2

multiload_applet_2_SOURCES = \
			global.h	\
			linux-proc.h	\
			load-graph.h	\
			linux-proc.c	\
			load-graph.c	\
			main.c	\
			properties.c	\
			netspeed.c netspeed.h \
			autoscaler.c	\
			autoscaler.h

multiload_applet_2_LDADD = \
				$(GNOME_APPLETS_LIBS) 	\
				$(GTOP_APPLETS_LIBS)	\
				$(GIO_UNIX_LIBS)	\
				$(LIBM)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = multiload.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST =							\
	org.gnome.applets.MultiLoadApplet.panel-applet.in.in	\
	$(service_in_files)					\
	$(schemas_in_files)					\
	$(ui_DATA)

uidir		= $(pkgdatadir)/ui
ui_DATA		= multiload-applet-menu.xml

appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.applets.MultiLoadApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.MultiLoadAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.MultiLoadAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif

-include $(top_srcdir)/git.mk
