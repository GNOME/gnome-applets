## Again, remove the comments on an applet you want to work on

gtop_applets = multiload
if BUILD_GTOP_APPLETS
gtop_SUBDIRS = $(gtop_applets)
endif

vfs_applets = \
	gtik	\
	gweather 
if BUILD_GNOME_VFS_APPLETS
vfs_SUBDIRS = $(vfs_applets)
endif

cdplayer_applets = \
	cdplayer
if APPLET_CDPLAYER
cdplayer_SUBDIRS = $(cdplayer_applets)
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

always_built_SUBDIRS = \
	drivemount	\
	charpick	\
	geyes		\
	mailcheck	\
	mini-commander	\
	gkb-new		\
	wireless	\
	stickynotes

SUBDIRS = \
	po			\
	screen-exec		\
	$(always_built_SUBDIRS)	\
	$(mixer_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(cdplayer_SUBDIRS)	\
	$(gtop_SUBDIRS)		\
	$(vfs_SUBDIRS)		\
	$(battstat_SUBDIR)	\
	$(accessx_status_SUBDIR)\
	$(gswitchit_SUBDIRS)

DIST_SUBDIRS = \
	po		\
	screen-exec	\
	mailcheck	\
	mixer		\
	modemlights	\
	cdplayer	\
	drivemount	\
	charpick	\
	geyes		\
	mini-commander	\
	gkb-new		\
	battstat	\
	multiload	\
	gtik		\
	gweather 	\
	mini-commander	\
	man		\
	wireless	\
	accessx-status	\
	stickynotes	\
	libgswitchit	\
	gswitchit

EXTRA_DIST = \
	COPYING-DOCS		\
	gnome-applets.spec.in	\
	gnome-applets.spec	\
	intltool-update.in	\
	intltool-extract.in	\
	intltool-merge.in	\
	xmldocs.make		\
	omf.make

