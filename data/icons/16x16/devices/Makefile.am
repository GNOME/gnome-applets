NULL =

themedir = $(pkgdatadir)/icons/hicolor
size = 16x16
context = devices

iconsdir = $(themedir)/$(size)/$(context)
icons_DATA = \
	$(NULL)

if BUILD_GTOP_APPLETS
icons_DATA += \
	netspeed-loopback.png \
	netspeed-plip.png \
	netspeed-ppp.png \
	$(NULL)
endif

EXTRA_DIST = \
	$(icons_DATA) \
	$(NULL)

-include $(top_srcdir)/git.mk
