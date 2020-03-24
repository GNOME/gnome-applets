NULL =

if APPLET_ACCESSX
accessx_status_SUBDIR = accessx-status
endif

if BUILD_CPUFREQ_APPLET
cpufreq_SUBDIR = cpufreq
endif

if HAVE_TRACKER_SEARCH_BAR
tracker_search_bar_SUBDIR = tracker-search-bar
endif

always_built_SUBDIRS =  \
	charpick	\
	battstat \
	brightness	\
	command		\
	drivemount	\
	inhibit		\
	geyes		\
	gweather \
	mini-commander \
	multiload \
	netspeed \
	sticky-notes	\
	timer \
	trash \
	window-buttons \
	window-picker \
	window-title \
	$(NULL)

SUBDIRS = \
	data \
	help \
	po			\
	$(always_built_SUBDIRS)	\
	$(vfs_SUBDIRS)		\
	$(accessx_status_SUBDIR)\
	$(cpufreq_SUBDIR) \
	$(tracker_search_bar_SUBDIR) \
	$(timer_SUBDIR) \
	$(NULL)

DIST_SUBDIRS = \
	data \
	help \
	po		\
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
	window-buttons	\
	window-picker	\
	window-title	\
	inhibit		\
	brightness	\
	tracker-search-bar \
	command		\
	timer	\
	$(NULL)

EXTRA_DIST = \
	COPYING-DOCS \
	$(NULL)

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-libpanel-applet-dir='$${datadir}/gnome-panel/distcheck/applets' \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_GETTEXT) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	`find "$(srcdir)/m4" -type f -name "*.m4" -print` \
	$(srcdir)/INSTALL \
	$(srcdir)/config.h.in~ \
	$(srcdir)/configure \
	$(NULL)

GITIGNOREFILES = $(PACKAGE)-\*.tar.{gz,bz2,xz}

-include $(top_srcdir)/git.mk
