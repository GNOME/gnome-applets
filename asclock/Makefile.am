INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel				\
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


asclock_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	asclock_applet.desktop	\
	asclock_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = asclock_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = asclock_applet.gnorba
