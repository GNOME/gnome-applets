NULL =

if BUILD_CPUFREQ_SELECTOR
selector_SUBDIR = cpufreq-selector
endif

SUBDIRS = $(selector_SUBDIR)

cpufreq_applet_CFLAGS = \
	-DCPUFREQ_MENU_UI_DIR=\""$(pkgdatadir)/ui"\" 		\
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	$(GNOME_APPLETS_CFLAGS) 				\
	$(LIBGLADE_CFLAGS)					\
	$(DBUS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libexec_PROGRAMS = cpufreq-applet

if HAVE_LIBCPUFREQ
cpufreq_files=cpufreq-monitor-libcpufreq.c    cpufreq-monitor-libcpufreq.h
endif

cpufreq_applet_SOURCES =  \
	cpufreq-applet.c        	cpufreq-applet.h        	\
	cpufreq-utils.c			cpufreq-utils.h			\
	cpufreq-prefs.c         	cpufreq-prefs.h         	\
	cpufreq-selector.c		cpufreq-selector.h		\
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
	$(LIBCPUFREQ_LIBS)		\
	$(DBUS_LIBS)

cpufreq_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

-include $(top_srcdir)/git.mk
