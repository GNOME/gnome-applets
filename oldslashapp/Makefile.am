INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = slash_applet

slash_applet_SOURCES = slashapp.c

slash_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	slash_applet.desktop

sysdir = $(datadir)/applets/Amusements
sys_DATA = slash_applet.desktop

