SUBDIRS = themes docs

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)             \
	-DGEYES_THEMES_DIR=\""$(datadir)/geyes/"\"

bin_PROGRAMS = geyes_applet

geyes_applet_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) $(INTLLIBS)

EXTRA_DIST = geyes_applet.desktop.in \
	     geyes_applet.gnorba \
	     geyes.h \
	     gnome-eyes.png

sysdir = $(datadir)/applets/Amusements
sys_predata = geyes_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = geyes_applet.gnorba 

themesdir = $(datadir)/geyes

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-eyes.png

man_MANS = geyes_applet.1

EXTRA_DIST = README.themes $(pixmap_DATA)	\
	$(gnorba_DATA) $(sys_DATA)		\
	$(sys_predata)				\
	$(man_MANS)


