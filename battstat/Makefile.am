NULL =

builder_DATA = battstat_applet.ui

ACPIINC= @ACPIINC@

if NEED_LIBAPM
if HAVE_LIBAPM
APMLIB = -lapm
APMDIR =
APMINC = 
else
APMLIB = apmlib/libapm.a
APMDIR = apmlib
APMINC = -I$(srcdir)/apmlib/
endif
else
APMLIB =
APMDIR =
APMINC = 
endif

SUBDIRS = help sounds $(APMDIR)

DIST_SUBDIRS = help sounds apmlib

libexec_PROGRAMS = battstat-applet-2

battstat_applet_2_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS)			\
	$(LIBNOTIFY_CFLAGS)			\
	$(UPOWER_CFLAGS)			\
	$(APMINC)				\
	$(ACPIINC)				\
	-DG_LOG_DOMAIN=\"battstat_applet\"	\
	-DBATTSTAT_MENU_UI_DIR=\""$(uidir)"\" \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

battstat_applet_2_SOURCES = \
	battstat.h \
	pixmaps.h \
	properties.c \
	battstat_applet.c \
	power-management.c \
	acpi-linux.c \
	acpi-linux.h \
	acpi-freebsd.c \
	acpi-freebsd.h \
	battstat-upower.c \
	battstat-upower.h

battstat_applet_2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

battstat_applet_2_LDADD = \
		     $(GNOME_APPLETS_LIBS) \
		     $(LIBNOTIFY_LIBS) \
		     $(UPOWER_LIBS) \
		     $(LIBM) \
		     $(APMLIB)

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.battstat.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.BattstatApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.BattstatAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.BattstatAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

uidir = $(pkgdatadir)/ui
ui_DATA = battstat-applet-menu.xml

EXTRA_DIST = 							\
	$(gsettings_schemas_in_in) \
	$(builder_DATA)						\
	$(ui_DATA)						\
	org.gnome.applets.BattstatApplet.panel-applet.in.in	\
	$(service_in_files)					\
	$(schemas_in_files)

-include $(top_srcdir)/git.mk
