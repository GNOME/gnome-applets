pixmapdir = $(datadir)/panel/

INCLUDES = 			\
	-I. -I$(srcdir)		\
	-I$(srcdir)/../../panel	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc


bin_PROGRAMS = netwatch_applet

netwatch_applet_SOURCES = netwatch.c

MICO_VERS = 2.0.5
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

netwatch_applet_LDADD = ../../panel/libpanel_applet.la \
                     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
                     $(GNOMEUI_LIBS) $(INTLLIBS)

netwatch_applet_LINK = $(CXXLINK)

EXTRA_DIST = netwatch_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = netwatch_applet.desktop

