gtop_applets = \
	cpumemusage	\
	diskusage	\
	multiload
if HAVE_LIBGTOP
gtop_SUBDIRS = $(gtop_applets)
endif

wordexp_applets = \
	screenshooter
if HAVE_WORDEXP
wordexp_SUBDIRS = $(wordexp_applets)
endif

xml_applets = \
	gnotes
if HAVE_LIBXML
xml_SUBDIRS = $(xml_applets)
endif

ghttp_applets = \
	gweather 
if HAVE_LIBGHTTP
ghttp_SUBDIRS = $(ghttp_applets)
endif

ghttp_vfs_applets = \
	gtik
if HAVE_LIBGHTTP
if HAVE_GNOME_VFS
ghttp_vfs_SUBDIRS = $(ghttp_vfs_applets)
endif
endif

ghttp_xml_applets = \
	slashapp
if HAVE_LIBXML
if HAVE_LIBGHTTP
ghttp_xml_SUBDIRS = $(ghttp_xml_applets)
endif
endif

esd_applets = \
	sound-monitor
if HAVE_LIBESD
esd_SUBDIRS = $(esd_applets)
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

always_built_SUBDIRS = \
	another_clock	\
	asclock		\
	battery		\
	charpick	\
	clockmail	\
	drivemount	\
	fifteen		\
	geyes		\
	gkb-new		\
	jbc		\
	life		\
	mini-commander	\
	quicklaunch	\
	tickastat	\
	webcontrol	\
	whereami	\
	odometer


SUBDIRS = \
	intl po macros 		\
	$(always_built_SUBDIRS)	\
	$(gtop_SUBDIRS)		\
	$(ghttp_SUBDIRS)	\
	$(ghttp_vfs_SUBDIRS)	\
	$(ghttp_xml_SUBDIRS)	\
	$(cdplayer_SUBDIRS)	\
	$(mixer_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(esd_SUBDIRS)		\
	$(wordexp_SUBDIRS)      \
	$(xml_SUBDIRS)		\
	omf-install

DIST_SUBDIRS = \
	intl po macros 		\
	$(always_built_SUBDIRS)	\
	clipboard		\
	$(gtop_applets)		\
	$(ghttp_applets)	\
	$(ghttp_vfs_applets)	\
	$(ghttp_xml_applets)	\
	$(cdplayer_applets)	\
	$(mixer_applets)	\
	$(modemlights_applets)	\
	$(esd_applets)		\
	$(wordexp_applets)      \
	$(xml_applets)		\
	omf-install

## to automatically rebuild aclocal.m4 if any of the macros in `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep

EXTRA_DIST = \
	COPYING-DOCS		\
	gnome-applets.spec.in	\
	gnome-applets.spec	\
	applet-docs.make	\
	sgmldocs.make		\
	xml-i18n-update.in	\
	xml-i18n-extract.in	\
	xml-i18n-merge.in

