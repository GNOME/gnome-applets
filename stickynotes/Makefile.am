NULL =

SUBDIRS = help pixmaps

ui_files = stickynotes-applet-menu.xml \
	stickynotes-note-menu.xml
builder_files = stickynotes-note.ui \
	stickynotes-delete.ui \
	stickynotes-delete-all.ui \
	stickynotes-preferences.ui \
	stickynotes-properties.ui
applet_in_files = org.gnome.applets.StickyNotesApplet.panel-applet.in
service_in_files = org.gnome.panel.applet.StickyNotesAppletFactory.service.in
gsettings_schemas_in_in = org.gnome.gnome-applets.stickynotes.gschema.xml.in.in

if BUILD_STICKYNOTES_APPLET
stickynotes_applet_CFLAGS = \
	-DSTICKYNOTES_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-I.					\
	-I$(srcdir)				\
	$(GNOME_APPLETS_CFLAGS)			\
	$(LIBWNCK_CFLAGS)			\
	$(XML2_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libexec_PROGRAMS = stickynotes_applet

stickynotes_applet_SOURCES = \
	util.h \
	util.c \
	stickynotes.h \
	stickynotes_callbacks.h \
	stickynotes_applet.h \
	stickynotes_applet_callbacks.h \
	stickynotes.c \
	stickynotes_callbacks.c \
	stickynotes_applet.c \
	stickynotes_applet_callbacks.c \
	gsettings.h

stickynotes_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

stickynotes_applet_LDADD =	\
	$(GNOME_APPLETS_LIBS)	\
	$(LIBWNCK_LIBS)		\
	$(X_LIBS)		\
	$(XML2_LIBS)

builderdir = $(pkgdatadir)/builder
builder_DATA = $(builder_files)

uidir = $(pkgdatadir)/ui
ui_DATA = $(ui_files)

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

appletdir   = $(LIBPANEL_APPLET_DIR)
applet_DATA = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir   = $(datadir)/dbus-1/services
service_DATA = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.StickyNotesAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

endif

EXTRA_DIST = \
	$(gsettings_schemas_in_in) \
	     $(builder_files) \
	     $(ui_files) \
	     $(schemas_in_files) \
             $(service_in_files) \
	     org.gnome.applets.StickyNotesApplet.panel-applet.in.in

-include $(top_srcdir)/git.mk
