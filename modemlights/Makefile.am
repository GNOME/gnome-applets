#SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 				\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = modemlights_applet2

modemlights_applet2_SOURCES = \
	modemlights.c	\
	modemlights.h	\
	properties.c

modemlights_applet2_LDADD = \
		     $(GNOME_APPLETS_LIBS)


serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_ModemLightsApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

EXTRA_DIST = \
	digits.xpm $(server_in_files)	

