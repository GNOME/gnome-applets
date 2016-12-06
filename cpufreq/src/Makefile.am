NULL =

if BUILD_CPUFREQ_SELECTOR
selector_SUBDIR = cpufreq-selector
endif

SUBDIRS = $(selector_SUBDIR)

cpu_frequency_applet_libdir = $(pkglibdir)
cpu_frequency_applet_lib_LTLIBRARIES = libcpu-frequency-applet.la

libcpu_frequency_applet_la_CPPFLAGS = \
	-DCPUFREQ_MENU_UI_DIR=\""$(pkgdatadir)/ui"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DCPUFREQ_PIXMAPS_DIR=\""$(pkgdatadir)/cpufreq-applet/pixmaps"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.cpu-frequency\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libcpu_frequency_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBGLADE_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libcpu_frequency_applet_la_SOURCES = \
	cpufreq-applet.c \
	cpufreq-applet.h \
	cpufreq-utils.c \
	cpufreq-utils.h \
	cpufreq-prefs.c \
	cpufreq-prefs.h \
	cpufreq-selector.c \
	cpufreq-selector.h \
	cpufreq-popup.c \
	cpufreq-popup.h \
	cpufreq-monitor.c \
	cpufreq-monitor.h \
	$(NULL)

libcpu_frequency_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libcpu_frequency_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBGLADE_LIBS) \
	$(LIBCPUFREQ_LIBS) \
	$(DBUS_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
