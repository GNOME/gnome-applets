## Again, remove the comments on an applet you want to work on

gtop_applets = multiload
if BUILD_GTOP_APPLETS
gtop_SUBDIRS = $(gtop_applets)
endif

vfs_applets = \
	gtik	\
	gweather \
	trashapplet
if BUILD_GNOME_VFS_APPLETS
vfs_SUBDIRS = $(vfs_applets)
endif

mixer_applets = \
	mixer
if APPLET_MIXER
mixer_SUBDIRS = $(mixer_applets)
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

if APPLET_GSWITCHIT
gswitchit_SUBDIRS = libgswitchit gswitchit
endif

if BUILD_CPUFREQ_APPLET
cpufreq_SUBDIR = cpufreq
endif

always_built_SUBDIRS = \
	charpick	\
	drivemount	\
	geyes		\
	mini-commander	\
	gkb-new		\
	stickynotes	\
	null_applet

SUBDIRS = \
	po			\
	$(always_built_SUBDIRS)	\
	$(mixer_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(gtop_SUBDIRS)		\
	$(vfs_SUBDIRS)		\
	$(battstat_SUBDIR)	\
	$(accessx_status_SUBDIR)\
	$(gswitchit_SUBDIRS)	\
	$(cpufreq_SUBDIR)

DIST_SUBDIRS = \
	po		\
	mixer		\
	modemlights	\
	drivemount	\
	charpick	\
	geyes		\
	mini-commander	\
	gkb-new		\
	battstat	\
	multiload	\
	gtik		\
	gweather 	\
	man		\
	accessx-status	\
	stickynotes	\
	libkbdraw	\
	libgswitchit	\
	gswitchit	\
	trashapplet	\
	cpufreq		\
	null_applet

EXTRA_DIST = \
	m4/ac-define-dir.m4		\
	m4/system-tools-backends.m4	\
	COPYING-DOCS			\
	gnome-applets.spec.in		\
	gnome-applets.spec		\
	intltool-update.in		\
	intltool-extract.in		\
	intltool-merge.in		\
	xmldocs.make			\
	omf.make			\
	gnome-doc-utils.make

DISTCHECK_CONFIGURE_FLAGS =	\
	--disable-scrollkeeper

# hack from sound-juicer to get around scrollkeeper's leaving a big
# mess behind.
distuninstallcheck_listfiles = 	\
        find -regex '.*/var/scrollkeeper/.*' -prune -or -type f -print

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = intltool-extract	\
		 intltool-merge		\
		 intltool-update	\
		 gnome-doc-utils.make
