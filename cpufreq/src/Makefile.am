if BUILD_CPUFREQ_SELECTOR
selector_SUBDIR = cpufreq-selector
endif

SUBDIRS = $(selector_SUBDIR)

INCLUDES = \
	-I$(top_srcdir)/intl					\
	$(GNOME_APPLETS_CFLAGS) 				\
	$(GNOME_VFS_APPLETS_CFLAGS) 				\
	$(LIBGLADE_CFLAGS)           				\
	-DICONDIR=\""$(datadir)/pixmaps"\"                      \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"                \
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED         \
	-DGNOME_DISABLE_DEPRECATED

libexec_PROGRAMS = cpufreq-applet

cpufreq_applet_SOURCES =  \
	cpufreq-applet.c        cpufreq-applet.h        \
	cpufreq-prefs.c         cpufreq-prefs.h         \
	cpufreq.c               cpufreq.h               \
	cpufreq-popup.c         cpufreq-popup.h

cpufreq_applet_LDADD =  \
	$(GNOME_APPLETS_LIBS)   	\
	$(GNOME_VFS_APPLETS_LIBS)	\
	$(LIBGLADE_LIBS)


	
