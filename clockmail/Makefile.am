INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = clockmail_applet

clockmail_applet_SOURCES = \
	clockmail.c	\
	clockmail.h	\
	properties.c	\
	theme.c		\
	rotate.c	\
	rotate.h


clockmail_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	clockmail_applet.desktop	\
	backgrnd.xpm			\
	backgrnd_tiny.xpm		\
	backgrnd_tiny_v.xpm		\
	digmed.xpm			\
	digsml.xpm			\
	mailpics.xpm			\
	button.xpm			\
	button_tiny.xpm			\
	button_tiny_v.xpm		\
	clockmaildata			\
	THEME-SPEC			\
	gnome-clockmail.png		\
	clockmail_applet.gnorba

clocksdir = $(datadir)/applets/Clocks
clocks_DATA = clockmail_applet.desktop

netdir = $(datadir)/applets/Network
net_DATA = clockmail_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = clockmail_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-clockmail.png

SUBDIRS = themes
