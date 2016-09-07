NULL =

if BUILD_CPUFREQ_SELECTOR
selector_SUBDIR = cpufreq-selector
endif

SUBDIRS = $(selector_SUBDIR)

cpu_frequency_applet_libdir = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)
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
	cpufreq-monitor-factory.c \
	cpufreq-monitor-factory.h \
	cpufreq-monitor-procfs.c \
	cpufreq-monitor-procfs.h \
	cpufreq-monitor-sysfs.c \
	cpufreq-monitor-sysfs.h \
	cpufreq-monitor-cpuinfo.c \
	cpufreq-monitor-cpuinfo.h \
	$(cpufreq_files) \
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
	$(NULL)

if HAVE_LIBCPUFREQ

cpufreq_files = \
    cpufreq-monitor-libcpufreq.c \
    cpufreq-monitor-libcpufreq.h \
    $(NULL)

endif

-include $(top_srcdir)/git.mk
