NULL =

themedir = $(datadir)/icons/hicolor
size = scalable
context = apps

iconsdir = $(themedir)/$(size)/$(context)
icons_DATA = \
	gnome-brightness-applet.svg \
	gnome-eyes-applet.svg \
	gnome-inhibit-applet.svg \
	gnome-sticky-notes-applet.svg \
	netspeed-applet.svg \
	$(NULL)

if BUILD_CPUFREQ_APPLET
icons_DATA += \
	gnome-cpu-frequency-applet.svg \
	$(NULL)
endif

EXTRA_DIST = \
	$(icons_DATA) \
	$(NULL)

-include $(top_srcdir)/git.mk
