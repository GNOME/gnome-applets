## Process this file with automake to produce Makefile.in

NULL=
SUBDIRS = docs

INCLUDES = 							\
	-I$(top_srcdir)/intl 					\
	$(GNOME_APPLETS_CFLAGS)					\
	$(LIBGLADE_CFLAGS)					\
	-DICONDIR=\""$(datadir)/pixmaps"\"			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED		\
	-DGNOME_DISABLE_DEPRECATED	\
	$(NULL)

libexec_PROGRAMS = wireless-applet

wireless_applet_SOURCES = 	\
	wireless-applet.c	\
	$(NULL)

wireless_applet_LDADD = 	\
	$(GNOME_APPLETS_LIBS) 	\
	$(LIBGLADE_LIBS)	\
	$(NULL)

gladedir = $(datadir)/wireless-applet/
glade_DATA = wireless-applet.glade

serverdir          = $(libdir)/bonobo/servers
server_in_files    = GNOME_Panel_WirelessApplet.server.in
server_in_in_files = $(server_in_files:.server.in=.server.in.in)
server_DATA        = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_in_files)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_WirelessApplet.xml

pixmapdir = $(datadir)/pixmaps/wireless-applet/
pixmap_DATA = no-link-0.png	\
        broken-0.png		\
	signal-1-40.png		\
	signal-41-60.png	\
	signal-61-80.png	\
	signal-81-100.png	\
	$(NULL)

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = wireless-applet.png

CLEANFILES = $(server_in_files) $(server_DATA) *.bak *.gladep

EXTRA_DIST = 				\
	$(server_in_in_files)		\
	$(glade_DATA)			\
	$(pixmap_DATA)			\
	$(icon_DATA)			\
	$(ui_DATA)			\
	$(NULL)

@INTLTOOL_SERVER_RULE@
