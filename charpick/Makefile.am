SUBDIRS = help

INCLUDES =					\
	-I.					\
	-I$(srcdir)				\
	-DCHARPICK_MENU_UI_DIR=\""$(uidir)"\"	\
	$(GNOME_APPLETS_CFLAGS)		\
	$(GUCHARMAP_CFLAGS)

libexec_PROGRAMS = charpick_applet2

charpick_applet2_SOURCES = charpick.c \
                          charpick.h \
                          properties.c

charpick_applet2_LDADD = \
		       $(GNOME_APPLETS_LIBS)	\
		       $(GUCHARMAP_LIBS)

appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.applets.CharpickerApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.CharpickerAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.CharpickerAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = charpick.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@
uidir = $(pkgdatadir)/ui
ui_DATA = charpick-applet-menu.xml

EXTRA_DIST =							\
	org.gnome.applets.CharpickerApplet.panel-applet.in.in	\
	$(service_in_files)					\
	$(ui_DATA)						\
	$(schemas_in_files)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif

-include $(top_srcdir)/git.mk
