cpufreqselectordir = $(bindir)

INCLUDES = -Wall $(CPUFREQ_SELECTOR_CFLAGS)

LDADD = $(CPUFREQ_SELECTOR_LIBS) -lpopt

bin_PROGRAMS = cpufreq-selector

install-exec-hook:
if CPUFREQ_SELECTOR_SUID
	chmod 4755 $(DESTDIR)$(bindir)/cpufreq-selector
endif

cpufreq_selector_SOURCES = \
	cpufreq.c               cpufreq.h        \
	cpufreq-sysfs.c         cpufreq-sysfs.h  \
	cpufreq-procfs.c        cpufreq-procfs.h \
	main.c

cpufreq_selector_LDADD = $(CPUFREQ_SELECTOR_LIBS) -lpopt

	
