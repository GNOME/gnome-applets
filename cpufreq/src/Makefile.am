if BUILD_CPUFREQ_SELECTOR
selector_SUBDIR = cpufreq-selector
endif

SUBDIRS = $(selector_SUBDIR)

INCLUDES = \
	$(GNOME_APPLETS_CFLAGS) 				\
	$(GNOME_VFS_APPLETS_CFLAGS) 				\
	$(LIBGLADE_CFLAGS)           				\
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED         \
	-DGNOME_DISABLE_DEPRECATED

libexec_PROGRAMS = cpufreq-applet

cpufreq_applet_SOURCES =  \
	cpufreq-applet.c        	cpufreq-applet.h        	\
	cpufreq-prefs.c         	cpufreq-prefs.h         	\
	cpufreq-popup.c                 cpufreq-popup.h			\
	cpufreq-monitor.c       	cpufreq-monitor.h	      	\
					cpufreq-monitor-protected.h	\
	cpufreq-monitor-factory.c	cpufreq-monitor-factory.h	\
	cpufreq-monitor-procfs.c	cpufreq-monitor-procfs.h	\
	cpufreq-monitor-sysfs.c	        cpufreq-monitor-sysfs.h	        \
	cpufreq-monitor-cpuinfo.c       cpufreq-monitor-cpuinfo.h

cpufreq_applet_LDADD =  \
	$(GNOME_APPLETS_LIBS)   	\
	$(GNOME_VFS_APPLETS_LIBS)	\
	$(LIBGLADE_LIBS)


	
