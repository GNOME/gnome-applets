SUBDIRS = themes docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 				\
	-DGEYES_THEMES_DIR=\""$(datadir)/geyes/"\"

bin_PROGRAMS = geyes_applet2

geyes_applet2_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet2_LDADD = \
	$(GNOME_APPLETS_LIBS)

EXTRA_DIST =  \
	     geyes.h \
	     gnome-eyes.png

sysdir = $(datadir)/applets/Amusements
sys_predata = geyes_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GeyesApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

themesdir = $(datadir)/geyes

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-eyes.png

EXTRA_DIST = README.themes $(pixmap_DATA)	\
	 $(server_in_files) $(sys_DATA) $(sys_predata)

