NULL =

timer_applet_libdir = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)
timer_applet_lib_LTLIBRARIES = libtimer-applet.la

libtimer_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	$(NULL)

libtimer_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtimer_applet_la_SOURCES = \
	timer-applet.c \
	$(NULL)

libtimer_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libtimer_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(NULL)

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.timer.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

EXTRA_DIST = \
	$(gsettings_schemas_in_in) \
	$(NULL)

CLEANFILES = \
	$(gsettings_SCHEMAS) \
	*.gschema.valid \
	$(NULL)

-include $(top_srcdir)/git.mk
