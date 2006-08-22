#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	$(LIBGLADE_CFLAGS)

libexec_PROGRAMS = modem_applet

modem_applet_SOURCES = \
	modem-applet.c \
	modem-applet.h

modem_applet_LDADD = 		\
	$(GNOME_APPLETS_LIBS)	\
	$(LIBGLADE_LIBS)	\
	$(MODEMLIGHTS_LIBS)

uidir	= $(datadir)/gnome-2.0/ui
ui_DATA	= GNOME_ModemLights.xml


serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_ModemLights.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

@INTLTOOL_SERVER_RULE@

glade_DATA = modemlights.glade

icons = \
	gnome-modem-monitor-applet.svg	  \
	gnome-modem-monitor-applet.16.png \
	gnome-modem-monitor-applet.32.png \
	gnome-modem-monitor-applet.22.png \
	gnome-modem-monitor-applet.24.png

EXTRA_DIST =				\
	GNOME_ModemLights.server.in.in	\
	$(icons)			\
	$(glade_DATA)			\
	$(ui_DATA)

uninstall-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" == "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		rm -f $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/$$FILE; \
	done

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
		mkdir -p $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(iconsdir)/hicolor/$$SIZE/apps/$$FILE; \
	done
