## Again, remove the comments on an applet you want to work on

gtop_applets = multiload
if BUILD_GTOP_APPLETS
gtop_SUBDIRS = $(gtop_applets)
endif

wordexp_applets = screenshooter
if HAVE_WORDEXP
wordexp_SUBDIRS = $(wordexp_applets)
endif

vfs_applets = \
	gtik	\
 	gweather 
if BUILD_GNOME_VFS_APPLETS
vfs_SUBDIRS = $(vfs_applets)
endif

zvt_applets = \
	mini-commander
if BUILD_ZVT_APPLETS
zvt_SUBDIRS = $(zvt_applets)
endif

## esd_applets = \
## 	sound-monitor
## if HAVE_LIBESD
## esd_SUBDIRS = $(esd_applets)
## endif
## 

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

always_built_SUBDIRS = \
	drivemount	\
	panel-menu	\
	charpick	\
	geyes		\
	mini-commander	\
	gkb-new		\
	battstat
##	whereami	

SUBDIRS = \
	po			\
	$(mixer_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(wordexp_SUBDIRS)      \
	$(cdplayer_SUBDIRS)	\
	$(always_built_SUBDIRS)	\
 	$(gtop_SUBDIRS)		\
	$(vfs_SUBDIRS)	\
	$(zvt_SUBDIRS)
## 	$(esd_SUBDIRS)		\
## 	omf-install

DIST_SUBDIRS = \
	po		\
	mixer		\
	modemlights	\
	screenshooter	\
	cdplayer	\
	drivemount	\
	panel-menu	\
	charpick	\
	geyes		\
	mini-commander	\
	gkb-new		\
	battstat	\
	multiload	\
	gtik		\
 	gweather 	\
	mini-commander	\
	man

EXTRA_DIST = \
	COPYING-DOCS		\
	gnome-applets.spec.in	\
	gnome-applets.spec	\
	applet-docs.make	\
	sgmldocs.make		\
	intltool-update.in	\
	intltool-extract.in	\
	intltool-merge.in	\
	message-of-doom

