NULL =

SUBDIRS = \
	charpick \
	drivemount \
	geyes \
	gweather \
	mini-commander \
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

if BUILD_GTOP_APPLETS
SUBDIRS += \
	multiload \
	netspeed \
	$(NULL)
endif

-include $(top_srcdir)/git.mk
