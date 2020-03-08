NULL =

cpufreqselectordir = $(bindir)

cpufreq_selector_CFLAGS = \
	$(CPUFREQ_SELECTOR_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

bin_PROGRAMS = cpufreq-selector

cpufreq_selector_SOURCES = \
	cpufreq-selector.c \
	cpufreq-selector.h \
	cpufreq-selector-service.c \
	cpufreq-selector-service.h \
	main.c \
	$(BUILT_SOURCES) \
	$(NULL)

cpufreq_selector_LDADD = \
	$(CPUFREQ_SELECTOR_LIBS) \
	$(LIBCPUFREQ_LIBS) \
	$(NULL)

cpufreq_selector_LDFLAGS = \
	-Wl,-z,now \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

dbus_services_in_files = org.gnome.CPUFreqSelector.service.in
polkit_in_files = org.gnome.cpufreqselector.policy.in

dbus_servicesdir = $(datadir)/dbus-1/system-services
dbus_confdir = $(sysconfdir)/dbus-1/system.d
polkitdir = $(datadir)/polkit-1/actions

cpufreq-selector-gen.h:
cpufreq-selector-gen.c: ../org.gnome.CPUFreqSelector.xml
	$(AM_V_GEN) $(GDBUS_CODEGEN) --c-namespace CPUFreq \
		--generate-c-code cpufreq-selector-gen \
		$(srcdir)/../org.gnome.CPUFreqSelector.xml

BUILT_SOURCES = \
	cpufreq-selector-gen.c \
	cpufreq-selector-gen.h \
	$(NULL)

dbus_services_DATA = $(dbus_services_in_files:.service.in=.service)
$(dbus_services_DATA): $(dbus_services_in_files)
	sed -e "s|\@BINDIR\@|$(DESTDIR)$(bindir)|" $< > $@
dbus_conf_DATA = org.gnome.CPUFreqSelector.conf

@INTLTOOL_POLICY_RULE@
polkit_DATA = $(polkit_in_files:.policy.in=.policy)

EXTRA_DIST = \
	$(dbus_services_in_files)	\
	org.gnome.CPUFreqSelector.conf	\
	$(polkit_in_files)

CLEANFILES = \
	$(BUILT_SOURCES)	\
	$(dbus_services_DATA)	\
	$(polkit_DATA)

-include $(top_srcdir)/git.mk
