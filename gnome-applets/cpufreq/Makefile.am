NULL =

SUBDIRS = cpufreq-selector

if BUILD_CPUFREQ_APPLET
noinst_LTLIBRARIES = libcpu-frequency-applet.la

libcpu_frequency_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.cpu-frequency\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libcpu_frequency_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
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
	cpufreq-popup.c \
	cpufreq-popup.h \
	cpufreq-monitor.c \
	cpufreq-monitor.h \
	$(BUILT_SOURCES) \
	$(NULL)

libcpu_frequency_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libcpu_frequency_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(LIBCPUFREQ_LIBS) \
	$(NULL)

cpufreq-selector-gen.h:
cpufreq-selector-gen.c: org.gnome.CPUFreqSelector.xml
	$(AM_V_GEN) $(GDBUS_CODEGEN) --c-namespace CPUFreq \
		--generate-c-code cpufreq-selector-gen \
		$(srcdir)/org.gnome.CPUFreqSelector.xml

BUILT_SOURCES = \
	cpufreq-selector-gen.c \
	cpufreq-selector-gen.h \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(NULL)
endif

EXTRA_DIST = \
	org.gnome.CPUFreqSelector.xml \
	$(NULL)

-include $(top_srcdir)/git.mk
