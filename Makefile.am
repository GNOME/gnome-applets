NULL =

gtop_applets = multiload netspeed
if BUILD_GTOP_APPLETS
gtop_SUBDIRS = $(gtop_applets)
endif

libgweather_applets = \
	gweather
if BUILD_LIBGWEATHER_APPLETS
libgweather_SUBDIRS = $(libgweather_applets)
endif

modemlights_applets = \
	modem-lights
if APPLET_MODEMLIGHTS
modemlights_SUBDIRS = $(modemlights_applets)
endif

if BUILD_BATTSTAT_APPLET
battstat_SUBDIR = battstat
endif

if APPLET_ACCESSX
accessx_status_SUBDIR = accessx-status
endif

if BUILD_CPUFREQ_APPLET
cpufreq_SUBDIR = cpufreq
endif

if BUILD_MINICOMMANDER_APPLET
minicommander_applet_SUBDIR = mini-commander
endif

if HAVE_TRACKER_SEARCH_BAR
tracker_search_bar_SUBDIR = tracker-search-bar
endif

if BUILD_TIMER
timer_SUBDIR = timer
endif

always_built_SUBDIRS =  \
	charpick	\
	brightness	\
	command		\
	drivemount	\
	inhibit		\
	invest-applet	\
	geyes		\
	sticky-notes	\
	trash \
	windowpicker \
	$(NULL)

SUBDIRS = \
	po			\
	$(always_built_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(gtop_SUBDIRS)		\
	$(libgweather_SUBDIRS)	\
	$(vfs_SUBDIRS)		\
	$(battstat_SUBDIR)	\
	$(accessx_status_SUBDIR)\
	$(cpufreq_SUBDIR) \
	$(minicommander_applet_SUBDIR) \
	$(tracker_search_bar_SUBDIR) \
	$(timer_SUBDIR) \
	$(NULL)

DIST_SUBDIRS = \
	po		\
	modem-lights	\
	drivemount	\
	charpick	\
	geyes		\
	mini-commander	\
	battstat	\
	multiload	\
	netspeed	\
	gweather 	\
	accessx-status	\
	sticky-notes	\
	trash	\
	cpufreq		\
	invest-applet	\
	windowpicker	\
	inhibit		\
	brightness	\
	tracker-search-bar \
	command		\
	timer	\
	$(NULL)

EXTRA_DIST = \
	MAINTAINERS			\
	COPYING-DOCS \
	$(NULL)

AM_DISTCHECK_CONFIGURE_FLAGS =	\
	--with-libpanel-applet-dir='$${datadir}/gnome-panel/distcheck/applets' \
	--enable-mini-commander

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	`find "$(srcdir)/m4" -type f -name "*.m4" -print` \
	$(srcdir)/INSTALL \
	$(srcdir)/config.h.in~ \
	$(srcdir)/configure \
	$(NULL)

GITIGNOREFILES = $(PACKAGE)-\*.tar.{gz,bz2,xz}

-include $(top_srcdir)/git.mk
