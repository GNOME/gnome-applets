SUBDIRS = themes default_theme docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DASCLOCK_THEMES_DIR=\""$(datadir)/asclock/"\"  \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DASCLOCK_GNOME -DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = asclock_applet

asclock_applet_SOURCES = \
	asclock.c	\
	asclock.h	\
	draw.c          \
        timezone.c      \
        dialogs.c	\
	config.c	\
	gnome_config.c	\
	rot.c		\
	symbols.c	\
	parser.c	\
	version.h


asclock_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Clocks
sys_predata = asclock_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = asclock_applet.gnorba

themesdir    = $(datadir)/asclock/

man_MANS = asclock_applet.1
 
EXTRA_DIST = $(gnorba_DATA)	\
	$(sys_DATA)		\
	$(sys_predata)		\
	$(man_MANS)
