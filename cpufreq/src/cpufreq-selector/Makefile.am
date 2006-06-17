cpufreqselectordir = $(bindir)

INCLUDES = $(CPUFREQ_SELECTOR_CFLAGS)

LDADD = $(CPUFREQ_SELECTOR_LIBS)

bin_PROGRAMS = cpufreq-selector

install-exec-hook:
if CPUFREQ_SELECTOR_SUID
	chmod 4755 $(DESTDIR)$(bindir)/cpufreq-selector
endif

if HAVE_LIBCPUFREQ
cpufreq_files=cpufreq-selector-libcpufreq.c        cpufreq-selector-libcpufreq.h
endif

cpufreq_selector_SOURCES = \
	cpufreq-selector.c               cpufreq-selector.h        \
	cpufreq-selector-sysfs.c         cpufreq-selector-sysfs.h  \
	cpufreq-selector-procfs.c        cpufreq-selector-procfs.h \
	$(cpufreq_files)					   \
	main.c

cpufreq_selector_LDADD = \
	$(CPUFREQ_SELECTOR_LIBS) \
	$(LIBCPUFREQ_LIBS)


