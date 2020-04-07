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
	mini-commander \
	multiload \
	netspeed \
	sticky-notes \
	timer \
	tracker-search-bar \
	trash \
	window-buttons \
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
	$(WARN_CFLAGS) \
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
	$(top_builddir)/gnome-applets/mini-commander/libmini-commander-applet.la \
	$(top_builddir)/gnome-applets/multiload/libmultiload-applet.la \
	$(top_builddir)/gnome-applets/netspeed/libnet-speed-applet.la \
	$(top_builddir)/gnome-applets/sticky-notes/libsticky-notes-applet.la \
	$(top_builddir)/gnome-applets/timer/libtimer-applet.la \
	$(top_builddir)/gnome-applets/trash/libtrash-applet.la \
	$(top_builddir)/gnome-applets/window-buttons/libwindow-buttons-applet.la \
	$(top_builddir)/gnome-applets/window-picker/libwindow-picker-applet.la \
	$(top_builddir)/gnome-applets/window-title/libwindow-title-applet.la \
	$(GNOME_PANEL_LIBS) \
	$(NULL)

if BUILD_CPUFREQ_APPLET
org_gnome_gnome_applets_la_LIBADD += \
	$(top_builddir)/gnome-applets/cpufreq/libcpu-frequency-applet.la \
	$(NULL)
endif

if HAVE_TRACKER_SEARCH_BAR
org_gnome_gnome_applets_la_LIBADD += \
	$(top_builddir)/gnome-applets/tracker-search-bar/libtracker-search-bar-applet.la \
	$(NULL)
endif

org_gnome_gnome_applets_la_LDFLAGS = \
	-module -avoid-version \
	$(WARN_LDFLAGS) \
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
	cpufreq/cpufreq-applet-menu.ui \
	cpufreq/cpufreq-preferences.ui \
	$(NULL)

tracker_search_bar_ui_files = \
	tracker-search-bar/tracker-search-bar-menu.ui \
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
	sticky-notes/icons/sticky-notes-stock-locked.png \
	sticky-notes/icons/sticky-notes-stock-unlocked.png \
	sticky-notes/icons/sticky-notes-stock-close.png \
	sticky-notes/icons/sticky-notes-stock-resize-se.png \
	sticky-notes/icons/sticky-notes-stock-resize-sw.png \
	window-picker/wp-about-logo.png \
	$(NULL)

ui_files = \
	accessx-status/accessx-status-applet-menu.ui \
	battstat/battstat-applet.ui \
	battstat/battstat-applet-menu.ui \
	brightness/brightness-applet-menu.ui \
	charpick/charpick-applet-menu.ui \
	command/command-applet-menu.ui \
	drivemount/drivemount-applet-menu.ui \
	geyes/geyes-applet-menu.ui \
	gweather/gweather-applet-menu.ui \
	inhibit/inhibit-applet-menu.ui \
	mini-commander/mini-commander.ui \
	mini-commander/mini-commander-applet-menu.ui \
	multiload/multiload-applet-menu.ui \
	netspeed/netspeed-menu.ui \
	sticky-notes/ui/sticky-notes-applet-menu.ui \
	sticky-notes/ui/sticky-notes-delete.ui \
	sticky-notes/ui/sticky-notes-delete-all.ui \
	sticky-notes/ui/sticky-notes-note.ui \
	sticky-notes/ui/sticky-notes-note-menu.ui \
	sticky-notes/ui/sticky-notes-preferences.ui \
	sticky-notes/ui/sticky-notes-properties.ui \
	timer/timer-applet-menu.ui \
	trash/trash-empty.ui \
	trash/trash-menu.ui \
	window-buttons/window-buttons.ui \
	window-buttons/window-buttons-menu.ui \
	window-picker/wp-menu.ui \
	window-picker/wp-preferences-dialog.ui \
	window-title/window-title.ui \
	window-title/window-title-menu.ui \
	$(NULL)

if BUILD_CPUFREQ_APPLET
icon_files += \
	$(cpufreq_icon_files)  \
	$(NULL)

ui_files += \
	$(cpufreq_ui_files) \
	$(NULL)
endif

if HAVE_TRACKER_SEARCH_BAR
ui_files += \
	$(tracker_search_bar_ui_files) \
	$(NULL)
endif

gnome-applets.gresource.xml: Makefile.am
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

ga-resources.c: gnome-applets.gresource.xml Makefile.am $(resource_files)
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

if !HAVE_TRACKER_SEARCH_BAR
EXTRA_DIST += \
	$(tracker_search_bar_ui_files) \
	$(NULL)
endif

-include $(top_srcdir)/git.mk
