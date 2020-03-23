NULL =

themedir = $(pkgdatadir)/icons/hicolor
size = 16x16
context = status

iconsdir = $(themedir)/$(size)/$(context)
icons_DATA = \
	gpm-brightness-lcd.png \
	gpm-brightness-lcd-disabled.png \
	gpm-brightness-lcd-invalid.png \
	gpm-inhibit.png \
	gpm-inhibit-invalid.png \
	gpm-uninhibit.png \
	$(NULL)

if BUILD_GTOP_APPLETS
icons_DATA += \
	netspeed-wireless-25.png \
	netspeed-wireless-50.png \
	netspeed-wireless-75.png \
	netspeed-wireless-100.png \
	$(NULL)
endif

EXTRA_DIST = \
	$(icons_DATA) \
	$(NULL)

-include $(top_srcdir)/git.mk
