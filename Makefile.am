# 'nb' means 'not built'
gtop_applets = \
	cpumemusage	\
	diskusage	\
	multiload
gtop_nb_applets = \
	netload
if HAVE_LIBGTOP
gtop_SUBDIRS = $(gtop_applets)
gtop_nb_SUBDIRS = $(gtop_nb_applets)
endif

xml_applets = \
	gnotes
if HAVE_LIBXML
xml_SUBDIRS = $(xml_applets)
endif

ghttp_applets = \
	slashapp	\
	gweather
ghttp_nb_applets = \
	bussign		\
	oldslashapp
if FALSE
if HAVE_LIBGHTTP
ghttp_SUBDIRS = $(ghttp_applets)
ghttp_nb_SUBDIRS = $(ghttp_nb_applets)
endif
endif

esd_applets = \
	sound-monitor
esd_nb_applets = \
	esd-manager
if HAVE_LIBESD
esd_SUBDIRS = $(esd_applets)
esd_nb_SUBDIRS = $(esd_nb_applets)
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
	gkb		\
	gnome-pager	\
	hal		\
	jbc		\
	life		\
	mini-commander	\
	tickastat	\
	webcontrol	\
	whereami

not_built_SUBDIRS = \
	batmon		\
	fvwm-pager	\
	gticker		\
	icewm-pager	\
	netwatch	\
	winlist


SUBDIRS = \
	intl po macros		\
	$(always_built_SUBDIRS)	\
	$(gtop_SUBDIRS)		\
	$(ghttp_SUBDIRS)	\
	$(cdplayer_SUBDIRS)	\
	$(mixer_SUBDIRS)	\
	$(modemlights_SUBDIRS)	\
	$(esd_SUBDIRS)		\
	$(xml_SUBDIRS)


DIST_SUBDIRS = \
	intl po macros		\
	$(always_built_SUBDIRS)	\
	$(not_built_SUBDIRS)	\
	$(gtop_applets)		\
	$(gtop_nb_applets)	\
	$(ghttp_applets)	\
	$(ghttp_nb_applets)	\
	$(cdplayer_applets)	\
	$(mixer_applets)	\
	$(modemlights_applets)	\
	$(esd_applets)		\
	$(esd_nb_applets)	\
	$(xml_applets)		\
	$(xml_nb_applets)

## to automatically rebuild aclocal.m4 if any of the macros in `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep
