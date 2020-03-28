NULL =

SUBDIRS = \
	brightness \
	command \
	gweather \
	timer \
	trash \
	window-picker \
	$(NULL)

gnome_applets_libdir = $(GNOME_PANEL_MODULES_DIR)
gnome_applets_lib_LTLIBRARIES = org.gnome.gnome-applets.la

org_gnome_gnome_applets_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\""org.gnome.gnome-applets"\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DLOCALE_DIR=\"$(localedir)\" \
	$(AM_CPPFLAGS) \
	$(NULL)

org_gnome_gnome_applets_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBGWEATHER_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

org_gnome_gnome_applets_la_SOURCES = \
	ga-module.c \
	$(BUILT_SOURCES) \
	$(NULL)

org_gnome_gnome_applets_la_LIBADD = \
	$(top_builddir)/gnome-applets/brightness/libbrightness-applet.la \
	$(top_builddir)/gnome-applets/command/libcommand-applet.la \
	$(top_builddir)/gnome-applets/gweather/libgweather-applet.la \
	$(top_builddir)/gnome-applets/timer/libtimer-applet.la \
	$(top_builddir)/gnome-applets/trash/libtrash-applet.la \
	$(top_builddir)/gnome-applets/window-picker/libwindow-picker-applet.la \
	$(GNOME_PANEL_LIBS) \
	$(NULL)

org_gnome_gnome_applets_la_LDFLAGS = \
	-module -avoid-version \
	$(AM_LDFLAGS) \
	$(NULL)

icon_files = \
	window-picker/wp-about-logo.png \
	$(NULL)

ui_files = \
	brightness/brightness-applet-menu.xml \
	command/command-applet-menu.xml \
	gweather/gweather-applet-menu.xml \
	timer/timer-applet-menu.xml \
	trash/trash-empty.ui \
	trash/trash-menu.xml \
	window-picker/wp-menu.xml \
	window-picker/wp-preferences-dialog.ui \
	$(NULL)

gnome-applets.gresource.xml: Makefile
	$(AM_V_GEN) echo '<?xml version="1.0" encoding="UTF-8"?>' > $@; \
		echo '<gresources>' >> $@; \
		echo "  <gresource prefix=\"$(GRESOURCE_PREFIX)/icons\">" >> $@; \
		for f in $(icon_files); do \
		b=`basename $$f`; \
		echo "    <file compressed=\"true\" alias=\"$$b\">$$f</file>" >> $@; \
		done; \
		echo '  </gresource>' >> $@; \
		echo "  <gresource prefix=\"$(GRESOURCE_PREFIX)/ui\">" >> $@; \
		for f in $(ui_files); do \
		b=`basename $$f`; \
		echo "    <file compressed=\"true\" preprocess=\"xml-stripblanks\" alias=\"$$b\">$$f</file>" >> $@; \
		done; \
		echo '  </gresource>' >> $@; \
		echo '</gresources>' >> $@;

resource_files = \
	$(shell $(GLIB_COMPILE_RESOURCES) \
		--sourcedir=$(srcdir) \
		--generate-dependencies \
		$(builddir)/gnome-applets.gresource.xml)

ga-resources.c: gnome-applets.gresource.xml $(resource_files)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) \
		--target=$@ --sourcedir=$(srcdir) \
		--generate --c-name ga $<

ga-resources.h: gnome-applets.gresource.xml
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) \
		--target=$@ --sourcedir=$(srcdir) \
		--generate --c-name ga $<

BUILT_SOURCES = \
	ga-resources.c \
	ga-resources.h \
	$(NULL)

CLEANFILES = \
	gnome-applets.gresource.xml \
	$(BUILT_SOURCES) \
	$(NULL)

EXTRA_DIST = \
	$(icon_files) \
	$(ui_files) \
	$(NULL)

-include $(top_srcdir)/git.mk
