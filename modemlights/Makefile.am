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

desktop_iconsdir   = $(iconsdir)/hicolor/48x48/apps
desktop_icons_DATA = gnome-modem.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_ModemLights.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

@INTLTOOL_SERVER_RULE@

gladedir = $(datadir)/modemlights-applet/
glade_DATA = modemlights.glade

EXTRA_DIST =					\
	GNOME_ModemLights.server.in.in		\
	$(desktop_icons_DATA)			\
	$(glade_DATA)				\
	$(ui_DATA)
