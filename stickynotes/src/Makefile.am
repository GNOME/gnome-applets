NULL =

if BUILD_STICKYNOTES_APPLET

stickynotes_applet_libdir = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)
stickynotes_applet_lib_LTLIBRARIES = libsticky-notes-applet.la

libsticky_notes_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-DSTICKYNOTES_MENU_UI_DIR=\""$(uidir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	$(NULL)

libsticky_notes_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(XML2_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libsticky_notes_applet_la_SOURCES = \
	gsettings.h \
	stickynotes.c \
	stickynotes.h \
	stickynotes_applet.c \
	stickynotes_applet.h \
	stickynotes_applet_callbacks.c \
	stickynotes_applet_callbacks.h \
	stickynotes_callbacks.c \
	stickynotes_callbacks.h \
	util.c \
	util.h \
	$(NULL)

libsticky_notes_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libsticky_notes_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBWNCK_LIBS) \
	$(X_LIBS) \
	$(XML2_LIBS) \
	$(NULL)

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.stickynotes.gschema.xml.in.in \
	$(NULL)

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

CLEANFILES = \
	$(gsettings_SCHEMAS) \
	*.gschema.valid \
	$(NULL)

endif

EXTRA_DIST = \
	$(gsettings_schemas_in_in) \
	$(NULL)

-include $(top_srcdir)/git.mk
