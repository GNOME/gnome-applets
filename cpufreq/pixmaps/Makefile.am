
cpufreq_pixmapdir = $(pixmapsdir)/cpufreq-applet
cpufreq_pixmap_DATA =		\
	cpufreq-100.png         \
	cpufreq-75.png          \
	cpufreq-50.png          \
	cpufreq-25.png          \
	cpufreq-na.png

icons = \
	gnome-cpu-frequency-applet.16.png \
	gnome-cpu-frequency-applet.22.png \
	gnome-cpu-frequency-applet.24.png \
	gnome-cpu-frequency-applet.32.png \
	gnome-cpu-frequency-applet.svg

EXTRA_DIST = \
	$(cpufreq_pixmap_DATA) \
	$(icons)

install-data-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" == "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		mkdir -p $(iconsdir)/hicolor/$$SIZE/apps/; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(iconsdir)/hicolor/$$SIZE/apps/$$FILE; \
	done
