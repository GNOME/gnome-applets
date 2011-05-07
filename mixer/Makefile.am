SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 			\
	-DMIXER_MENU_UI_DIR=\""$(uidir)"\"	\
	$(MIXER_CFLAGS)				\
	$(GNOME_APPLETS_CFLAGS)			\
	$(WARN_CFLAGS)

libexec_PROGRAMS = mixer_applet2

mixer_applet2_SOURCES = \
	applet.c \
	dock.c \
	load.c \
	preferences.c

noinst_HEADERS = \
	applet.h \
	dock.h \
	keys.h \
	preferences.h

mixer_applet2_LDADD = 					\
	$(MIXER_LIBS) 					\
	$(GNOME_APPLETS_LIBS)

appletdir       = $(datadir)/gnome-panel/4.0/applets
applet_in_files = org.gnome.applets.MixerApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.MixerAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.MixerAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

uidir   = $(pkgdatadir)/ui
ui_DATA = mixer-applet-menu.xml

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = mixer.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = 							\
	org.gnome.applets.MixerApplet.panel-applet.in.in	\
	$(service_in_files)					\
	$(ui_DATA) 						\
	$(schemas_in_files)

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif

-include $(top_srcdir)/git.mk
