INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc

bin_PROGRAMS = printer_applet

printer_applet_SOURCES = printer.c

printer_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

printer_applet_LINK = $(CXXLINK)

BITMAPS = print.xpm

EXTRA_DIST = printer.desktop $BITMAPS

sysdir = $(datadir)/applets/Utility
sys_DATA = printer.desktop
