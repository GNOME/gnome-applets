INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = clockmail_applet

clockmail_applet_SOURCES = \
	clockmail.c	\
	clockmail.h	\
	properties.c	\
	theme.c


clockmail_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	clockmail_applet.desktop	\
	backgrnd.xpm			\
	digmed.xpm			\
	digsml.xpm			\
	mailpics.xpm			\
	button.xpm			\
	clockmaildata			\
	THEME-SPEC			\
	clockmail_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = clockmail_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = clockmail_applet.gnorba
