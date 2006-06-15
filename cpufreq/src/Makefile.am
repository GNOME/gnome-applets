if BUILD_CPUFREQ_SELECTOR
selector_SUBDIR = cpufreq-selector
endif

SUBDIRS = $(selector_SUBDIR)

INCLUDES = \
	$(GNOME_APPLETS_CFLAGS) 				\
	$(LIBGLADE_CFLAGS)

libexec_PROGRAMS = cpufreq-applet

if HAVE_LIBCPUFREQ
cpufreq_files=cpufreq-monitor-libcpufreq.c    cpufreq-monitor-libcpufreq.h
endif

cpufreq_applet_SOURCES =  \
	cpufreq-applet.c        	cpufreq-applet.h        	\
	cpufreq-utils.c			cpufreq-utils.h			\
	cpufreq-prefs.c         	cpufreq-prefs.h         	\
	cpufreq-popup.c                 cpufreq-popup.h			\
	cpufreq-monitor.c       	cpufreq-monitor.h	      	\
	cpufreq-monitor-factory.c	cpufreq-monitor-factory.h	\
	cpufreq-monitor-procfs.c	cpufreq-monitor-procfs.h	\
	cpufreq-monitor-sysfs.c	        cpufreq-monitor-sysfs.h	        \
	$(cpufreq_files)						\
	cpufreq-monitor-cpuinfo.c       cpufreq-monitor-cpuinfo.h

cpufreq_applet_LDADD =  \
	$(GNOME_APPLETS_LIBS)   	\
	$(LIBGLADE_LIBS)		\
	$(LIBCPUFREQ_LIBS)


	
