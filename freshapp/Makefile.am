INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = fresh_applet

fresh_applet_SOURCES = freshapp.c

slash_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	fresh_applet.desktop

sysdir = $(datadir)/applets/Amusements
sys_DATA = fresh_applet.desktop

