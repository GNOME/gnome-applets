SUBDIRS = docs themes

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	@PIXBUF_CFLAGS@

bin_PROGRAMS = clockmail_applet

clockmail_applet_SOURCES = \
	button.c	\
	button.h	\
	clock.c		\
	clock.h		\
	clockmail.c	\
	clockmail.h	\
	def_theme.c	\
	def_theme.h	\
	display.c	\
	display.h	\
	item.c		\
	item.h		\
	number.c	\
	number.h	\
	pixbuf_ops.c	\
	pixbuf_ops.h	\
	properties.c	\
	skin.c		\
	skin.h		\
	theme.c		\
	theme.h		\
	typedefs.h	\
	update.c	\
	update.h

clockmail_applet_LDADD = \
		$(GNOME_LIBDIR)		\
		$(GNOME_APPLETS_LIBS) $(INTLLIBS)	\
		@PIXBUF_LIBS@

clocksdir = $(datadir)/applets/Clocks
clocks_in_files = clockmail_applet.desktop.in
clocks_DATA = $(clocks_in_files:.desktop.in=.desktop)

sysdir = $(datadir)/applets/Network

sys_predata = clockmail_applet.desktop.in

sys_DATA = $(sys_predata:.desktop.in=.desktop)

@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = clockmail_applet.gnorba

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-clockmail.png

man_MANS = clockmail_applet.1

EXTRA_DIST = \
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
	$(gnorba_DATA) $(sys_DATA)	\
	$(sys_predata) $(man_MANS)


