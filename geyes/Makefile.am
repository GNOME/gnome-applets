SUBDIRS = themes

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)             \
	-DGEYES_THEMES_DIR=\""$(datadir)/geyes/"\"

bin_PROGRAMS = geyes_applet

geyes_applet_SOURCES =	\
        geyes.c \
	themes.c \
	geyes.h

geyes_applet_LDADD = $(GNOME_LIBDIR) $(GNOME_APPLETS_LIBS)

EXTRA_DIST = geyes_applet.desktop \
	     geyes_applet.gnorba \
	     geyes.h

sysdir = $(datadir)/applets/Amusements
sys_DATA = geyes_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = geyes_applet.gnorba 

themesdir = $(datadir)/geyes/

EXTRA_DIST = README.themes $(sys_DATA) $(gnorba_DATA)
