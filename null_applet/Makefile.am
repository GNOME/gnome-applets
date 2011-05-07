applet_in_files = org.gnome.applets.NullApplet.panel-applet.in
service_in_files = org.gnome.panel.applet.NullAppletFactory.service.in

INCLUDES = 			\
	-I.			\
	-I$(srcdir)		\
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS)

libexec_PROGRAMS = null_applet

null_applet_SOURCES = null_applet.c null_applet.h

null_applet_LDADD = $(GNOME_APPLETS_LIBS)

appletdir = $(datadir)/gnome-panel/4.0/applets
applet_DATA = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
	    -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	    -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
	    $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir   = $(datadir)/dbus-1/services
service_DATA = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.NullAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
	    -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	    $< > $@

CLEANFILES = $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

EXTRA_DIST = \
	$(service_in_files) \
	org.gnome.applets.NullApplet.panel-applet.in.in

-include $(top_srcdir)/git.mk
