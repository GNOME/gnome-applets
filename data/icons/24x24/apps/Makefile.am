NULL =

themedir = $(datadir)/icons/hicolor
size = 24x24
context = apps

iconsdir = $(themedir)/$(size)/$(context)
icons_DATA = \
	gnome-brightness-applet.png \
	gnome-eyes-applet.png \
	gnome-inhibit-applet.png \
	gnome-sticky-notes-applet.png \
	netspeed-applet.png \
	$(NULL)

if BUILD_CPUFREQ_APPLET
icons_DATA += \
	gnome-cpu-frequency-applet.png \
	$(NULL)
endif

EXTRA_DIST = \
	$(icons_DATA)	\
	$(NULL)

-include $(top_srcdir)/git.mk
