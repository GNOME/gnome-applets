## Again, remove the comments on an applet you want to work on

gtop_applets = multiload
if BUILD_GTOP_APPLETS
gtop_SUBDIRS = $(gtop_applets)
endif

wordexp_applets = screenshooter
if HAVE_WORDEXP
wordexp_SUBDIRS = $(wordexp_applets)
endif

## ghttp_applets = \
## 	gtik
## 	gweather 
## 	slashapp
## if HAVE_LIBGHTTP
## ghttp_SUBDIRS = $(ghttp_applets)
## endif
## 
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
##	fifteen		\
##	odometer	\
##	geyes		\
	mini-commander	\
##	life		\
	gkb-new		
##	whereami	
## 	another_clock	\
## 	asclock		\
## 	clockmail	\
## 	drivemount	\
## 	gnotes		\
## 	jbc		\
## 	quicklaunch	\
## 	tickastat	\
## 	webcontrol	\
## 
## 
SUBDIRS = \
	po			\
	$(mixer_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(wordexp_SUBDIRS)      \
	$(cdplayer_SUBDIRS)	\
	$(always_built_SUBDIRS)	\
 	$(gtop_SUBDIRS)		
## 	$(ghttp_SUBDIRS)	\
## 	$(esd_SUBDIRS)		\
## 	omf-install
## 
## DIST_SUBDIRS = \
##	po 			\
## 	$(always_built_SUBDIRS)	\
## 	clipboard		\
## 	$(gtop_applets)		\
## 	$(ghttp_applets)	\
## 	$(cdplayer_applets)	\
## 	$(mixer_applets)	\
## 	$(modemlights_applets)	\
## 	$(esd_applets)		\
## 	$(wordexp_applets)      
##	omf-install

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

