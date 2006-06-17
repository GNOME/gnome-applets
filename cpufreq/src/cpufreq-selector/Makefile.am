cpufreqselectordir = $(bindir)

INCLUDES = $(CPUFREQ_SELECTOR_CFLAGS)

LDADD = $(CPUFREQ_SELECTOR_LIBS)

bin_PROGRAMS = cpufreq-selector

install-exec-hook:
if CPUFREQ_SELECTOR_SUID
	chmod 4755 $(DESTDIR)$(bindir)/cpufreq-selector
endif

cpufreq_selector_SOURCES = \
	cpufreq-selector.c               cpufreq-selector.h        \
	cpufreq-selector-sysfs.c         cpufreq-selector-sysfs.h  \
	cpufreq-selector-procfs.c        cpufreq-selector-procfs.h \
	main.c

cpufreq_selector_LDADD = $(CPUFREQ_SELECTOR_LIBS)

