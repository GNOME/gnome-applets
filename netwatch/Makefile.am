# trick Automake into defining $(CXXLINK)
if FALSE
BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc
endif

pixmapdir = $(datadir)/panel/

INCLUDES = 			\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I. -I$(srcdir)		\
	-I$(srcdir)/../../panel	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = netwatch_applet

netwatch_applet_SOURCES = netwatch.c


netwatch_applet_LDADD = ../../panel/libpanel_applet.la \
                     $(GNOME_LIBDIR) $(ORB_LIBS)      \
                     $(GNOMEUI_LIBS) $(INTLLIBS)

netwatch_applet_LINK = $(CXXLINK)

EXTRA_DIST = \
	netwatch_applet.desktop		\
	netwatch-modem-down.xpm		\
	netwatch-modem-up.xpm		\
	netwatch-net-down.xpm		\
	netwatch-net-up.xpm

sysdir = $(datadir)/applets/Network
sys_DATA = netwatch_applet.desktop

