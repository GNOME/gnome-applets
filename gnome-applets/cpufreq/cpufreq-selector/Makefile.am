NULL =

dbus_services_in_files = org.gnome.CPUFreqSelector.service.in
dbus_conf_files = org.gnome.CPUFreqSelector.conf
polkit_in_files = org.gnome.cpufreqselector.policy.in

if BUILD_CPUFREQ_SELECTOR
bin_PROGRAMS = cpufreq-selector

cpufreq_selector_CFLAGS = \
	$(CPUFREQ_SELECTOR_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

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

dbus_servicesdir = $(datadir)/dbus-1/system-services
dbus_services_DATA = $(dbus_services_in_files:.service.in=.service)

$(dbus_services_DATA): $(dbus_services_in_files)
	sed -e "s|\@BINDIR\@|$(DESTDIR)$(bindir)|" $< > $@

dbus_confdir = $(datadir)/dbus-1/system.d
dbus_conf_DATA = $(dbus_conf_files)

polkitdir = $(datadir)/polkit-1/actions
polkit_DATA = $(polkit_in_files:.policy.in=.policy)

$(polkit_DATA): %.policy: %.policy.in
	$(AM_V_GEN) $(MSGFMT) --xml --template $< -o $@-t \
		-d $(top_srcdir)/po && mv $@-t $@

cpufreq-selector-gen.h:
cpufreq-selector-gen.c: ../org.gnome.CPUFreqSelector.xml
	$(AM_V_GEN) $(GDBUS_CODEGEN) --c-namespace CPUFreq \
		--generate-c-code cpufreq-selector-gen \
		$(srcdir)/../org.gnome.CPUFreqSelector.xml

BUILT_SOURCES = \
	cpufreq-selector-gen.c \
	cpufreq-selector-gen.h \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(dbus_services_DATA) \
	$(polkit_DATA) \
	$(NULL)
endif

EXTRA_DIST = \
	$(dbus_services_in_files) \
	$(dbus_conf_files) \
	$(polkit_in_files) \
	$(NULL)

-include $(top_srcdir)/git.mk
