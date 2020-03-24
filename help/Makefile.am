NULL =

SUBDIRS = \
	battstat \
	charpick \
	drivemount \
	geyes \
	gweather \
	mini-commander \
	multiload \
	netspeed \
	sticky-notes \
	trash \
	$(NULL)

if APPLET_ACCESSX
SUBDIRS += \
	accessx-status \
	$(NULL)
endif

if BUILD_CPUFREQ_APPLET
SUBDIRS += \
	cpufreq \
	$(NULL)
endif

-include $(top_srcdir)/git.mk
