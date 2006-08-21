## Process this file with automake to produce Makefile.in

stickynotes_applet_pixmapsdir = $(datadir)/pixmaps/stickynotes
stickynotes_applet_pixmaps_DATA = \
	locked.png \
	unlocked.png \
	close.png \
	resize_se.png \
	resize_sw.png

icons = \
	gnome-sticky-notes-applet.16.png \
	gnome-sticky-notes-applet.32.png \
	gnome-sticky-notes-applet.22.png \
	gnome-sticky-notes-applet.24.png \
	gnome-sticky-notes-applet.svg

EXTRA_DIST = \
	$(stickynotes_applet_pixmaps_DATA) \
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
