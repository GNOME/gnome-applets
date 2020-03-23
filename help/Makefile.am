NULL =

SUBDIRS = \
	charpick \
	drivemount \
	geyes \
	sticky-notes \
	trash \
	$(NULL)

if APPLET_ACCESSX
SUBDIRS += \
	accessx-status \
	$(NULL)
endif

if BUILD_BATTSTAT_APPLET
SUBDIRS += \
	battstat \
	$(NULL)
endif

if BUILD_CPUFREQ_APPLET
SUBDIRS += \
	cpufreq \
	$(NULL)
endif

if BUILD_LIBGWEATHER_APPLETS
SUBDIRS += \
	gweather \
	$(NULL)
endif

if BUILD_MINICOMMANDER_APPLET
SUBDIRS += \
	mini-commander \
	$(NULL)
endif

if BUILD_GTOP_APPLETS
SUBDIRS += \
	multiload \
	netspeed \
	$(NULL)
endif

-include $(top_srcdir)/git.mk
