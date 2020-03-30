NULL =

SUBDIRS = \
	accessx-status \
	battstat \
	brightness \
	charpick \
	command \
	cpufreq \
	drivemount \
	geyes \
	gweather \
	inhibit \
	netspeed \
	timer \
	trash \
	window-picker \
	window-title \
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
	$(top_builddir)/gnome-applets/accessx-status/libaccessx-status-applet.la \
	$(top_builddir)/gnome-applets/battstat/libbattery-status-applet.la \
	$(top_builddir)/gnome-applets/brightness/libbrightness-applet.la \
	$(top_builddir)/gnome-applets/charpick/libcharacter-picker-applet.la \
	$(top_builddir)/gnome-applets/command/libcommand-applet.la \
	$(top_builddir)/gnome-applets/drivemount/libdrive-mount-applet.la \
	$(top_builddir)/gnome-applets/geyes/libgeyes-applet.la \
	$(top_builddir)/gnome-applets/gweather/libgweather-applet.la \
	$(top_builddir)/gnome-applets/inhibit/libinhibit-applet.la \
	$(top_builddir)/gnome-applets/netspeed/libnet-speed-applet.la \
	$(top_builddir)/gnome-applets/timer/libtimer-applet.la \
	$(top_builddir)/gnome-applets/trash/libtrash-applet.la \
	$(top_builddir)/gnome-applets/window-picker/libwindow-picker-applet.la \
	$(top_builddir)/gnome-applets/window-title/libwindow-title-applet.la \
	$(GNOME_PANEL_LIBS) \
	$(NULL)

if BUILD_CPUFREQ_APPLET
org_gnome_gnome_applets_la_LIBADD += \
	$(top_builddir)/gnome-applets/cpufreq/libcpu-frequency-applet.la \
	$(NULL)
endif

org_gnome_gnome_applets_la_LDFLAGS = \
	-module -avoid-version \
	$(AM_LDFLAGS) \
	$(NULL)

cpufreq_icon_files = \
	cpufreq/icons/cpufreq-100.png \
	cpufreq/icons/cpufreq-75.png \
	cpufreq/icons/cpufreq-50.png \
	cpufreq/icons/cpufreq-25.png \
	cpufreq/icons/cpufreq-na.png \
	$(NULL)

cpufreq_ui_files = \
	cpufreq/cpufreq-applet-menu.xml \
	cpufreq/cpufreq-preferences.ui \
	$(NULL)

icon_files = \
	accessx-status/icons/ax-applet.png \
	accessx-status/icons/ax-key-base.png \
	accessx-status/icons/ax-key-inverse.png \
	accessx-status/icons/ax-key-none.png \
	accessx-status/icons/ax-key-yes.png \
	accessx-status/icons/ax-key-no.png \
	accessx-status/icons/mousekeys-base.png \
	accessx-status/icons/mousekeys-pressed-left.png \
	accessx-status/icons/mousekeys-pressed-middle.png \
	accessx-status/icons/mousekeys-pressed-right.png \
	accessx-status/icons/mousekeys-default-left.png \
	accessx-status/icons/mousekeys-default-middle.png \
	accessx-status/icons/mousekeys-default-right.png \
	accessx-status/icons/sticky-shift-none.png \
	accessx-status/icons/sticky-shift-latched.png \
	accessx-status/icons/sticky-shift-locked.png \
	accessx-status/icons/sticky-ctrl-none.png \
	accessx-status/icons/sticky-ctrl-latched.png \
	accessx-status/icons/sticky-ctrl-locked.png \
	accessx-status/icons/sticky-alt-none.png \
	accessx-status/icons/sticky-alt-latched.png \
	accessx-status/icons/sticky-alt-locked.png \
	accessx-status/icons/sticky-meta-latched.png \
	accessx-status/icons/sticky-meta-locked.png \
	accessx-status/icons/sticky-meta-none.png \
	accessx-status/icons/sticky-hyper-latched.png \
	accessx-status/icons/sticky-hyper-locked.png \
	accessx-status/icons/sticky-hyper-none.png \
	accessx-status/icons/sticky-super-latched.png \
	accessx-status/icons/sticky-super-locked.png \
	accessx-status/icons/sticky-super-none.png \
	accessx-status/icons/ax-slowkeys.png \
	accessx-status/icons/ax-slowkeys-pending.png \
	accessx-status/icons/ax-slowkeys-yes.png \
	accessx-status/icons/ax-slowkeys-no.png \
	accessx-status/icons/ax-bouncekeys.png \
	window-picker/wp-about-logo.png \
	$(NULL)

ui_files = \
	accessx-status/accessx-status-applet-menu.xml \
	battstat/battstat-applet.ui \
	battstat/battstat-applet-menu.xml \
	brightness/brightness-applet-menu.xml \
	charpick/charpick-applet-menu.xml \
	command/command-applet-menu.xml \
	drivemount/drivemount-applet-menu.xml \
	geyes/geyes-applet-menu.xml \
	gweather/gweather-applet-menu.xml \
	inhibit/inhibit-applet-menu.xml \
	netspeed/netspeed-menu.xml \
	timer/timer-applet-menu.xml \
	trash/trash-empty.ui \
	trash/trash-menu.xml \
	window-picker/wp-menu.xml \
	window-picker/wp-preferences-dialog.ui \
	window-title/window-title.ui \
	window-title/window-title-menu.xml \
	$(NULL)

if BUILD_CPUFREQ_APPLET
icon_files += \
	$(cpufreq_icon_files)  \
	$(NULL)

ui_files += \
	$(cpufreq_ui_files) \
	$(NULL)
endif

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

if !BUILD_CPUFREQ_APPLET
EXTRA_DIST += \
	$(cpufreq_icon_files) \
	$(cpufreq_ui_files) \
	$(NULL)
endif

-include $(top_srcdir)/git.mk
