
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
	modemlights
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

if BUILD_INVEST_APPLET
invest_applet_SUBDIR = invest-applet
endif

if BUILD_MINICOMMANDER_APPLET
minicommander_applet_SUBDIR = mini-commander
endif

if HAVE_TRACKER_SEARCH_BAR
tracker_search_bar_SUBDIR = tracker-search-bar
endif

always_built_SUBDIRS =  \
	charpick	\
	brightness	\
	drivemount	\
	inhibit		\
	geyes		\
	stickynotes	\
	trashapplet \
	windowpicker

SUBDIRS = \
	po			\
	$(always_built_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(gtop_SUBDIRS)		\
	$(libgweather_SUBDIRS)	\
	$(vfs_SUBDIRS)		\
	$(battstat_SUBDIR)	\
	$(accessx_status_SUBDIR)\
	$(invest_applet_SUBDIR)	\
	$(cpufreq_SUBDIR) \
	$(minicommander_applet_SUBDIR) \
	$(tracker_search_bar_SUBDIR)

DIST_SUBDIRS = \
	po		\
	modemlights	\
	drivemount	\
	charpick	\
	geyes		\
	mini-commander	\
	battstat	\
	multiload	\
	netspeed	\
	gweather 	\
	man		\
	accessx-status	\
	stickynotes	\
	trashapplet	\
	cpufreq		\
	invest-applet	\
	windowpicker	\
	inhibit		\
	brightness	\
	tracker-search-bar

EXTRA_DIST = \
	MAINTAINERS			\
	COPYING-DOCS

DISTCHECK_CONFIGURE_FLAGS =	\
	--with-libpanel-applet-dir='$${datadir}/gnome-panel/distcheck/applets'
	--enable-mini-commander

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
	$(srcdir)/INSTALL \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/compile \
	$(srcdir)/config.guess \
	$(srcdir)/config.h.in \
	$(srcdir)/config.h.in~ \
	$(srcdir)/config.sub \
	$(srcdir)/depcomp \
	$(srcdir)/install-sh \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	$(srcdir)/mkinstalldirs \
	`find "$(srcdir)" -type f -name Makefile.in -print` \
	$(srcdir)/configure \
	$(srcdir)/gtk-doc.make \
	$(srcdir)/m4/gtk-doc.m4 \
	$(srcdir)/m4/intltool.m4 \
	$(srcdir)/m4/libtool.m4 \
	$(srcdir)/m4/ltoptions.m4 \
	$(srcdir)/m4/ltsugar.m4 \
	$(srcdir)/m4/ltversion.m4 \
	$(srcdir)/m4/lt~obsolete.m4 \
	$(srcdir)/py-compile

-include $(top_srcdir)/git.mk
