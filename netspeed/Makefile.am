SUBDIRS = help icons po src

appletdir = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.panel.Netspeed.panel-applet.in
applet_DATA = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
		-e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
		-e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
		$< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir = $(datadir)/dbus-1/services
service_in_files = org.gnome.panel.applet.NetspeedAppletFactory.service.in
service_DATA = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.NetspeedAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
		-e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
		$< > $@

@INTLTOOL_SERVICE_RULE@

distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper

spec = $(PACKAGE).spec
EXTRA_DIST = org.gnome.panel.Netspeed.panel-applet.in.in \
			 $(service_in_files) \
             $(spec) \
             gnome-doc-utils.make \
             intltool-update.in \
             intltool-merge.in \
             intltool-extract.in

all-local: $(spec) $(service_DATA)

CLEANFILES = \
			 $(applet_DATA) \
			 $(applet_DATA).in \
			 $(service_DATA)
DISTCLEANFILES = \
                 gnome-doc-utils.make \
                 intltool-extract \
                 intltool-merge \
                 intltool-update

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper
