NULL =

themedir = $(datadir)/icons/hicolor
size = 48x48
context = apps

iconsdir = $(themedir)/$(size)/$(context)
icons_DATA = \
	gnome-brightness-applet.png \
	gnome-inhibit-applet.png \
	netspeed-applet.png \
	$(NULL)

if APPLET_ACCESSX
icons_DATA += \
	ax-applet.png \
	$(NULL)
endif

EXTRA_DIST = \
	$(icons_DATA) \
	$(NULL)

-include $(top_srcdir)/git.mk
