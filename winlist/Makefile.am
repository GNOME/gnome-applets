INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = winlist_applet

winlist_applet_SOURCES = \
	winlist.c	\
	winlist.h	\
	fvwm-winlist.c	\
	fvwmlib.h	\
	module.h

winlist_applet_LDADD = \
		     $(GNOME_LIBDIR)	\
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)

EXTRA_DIST = winlist_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = winlist_applet.desktop

