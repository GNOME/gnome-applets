# trick Automake into defining $(CXXLINK)
if FALSE
BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc
endif

INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = fifteen_applet

fifteen_applet_SOURCES = fifteen.c

fifteen_applet_LDADD = ../../panel/libpanel_applet.la	\
		       $(GNOME_LIBDIR) $(MICO_LIBS)	\
		       $(GNOMEUI_LIBS) $(INTLLIBS) 

fifteen_applet_LINK = $(CXXLINK)

EXTRA_DIST = \
	fifteen_applet.desktop

sysdir = $(datadir)/applets/Amusements
sys_DATA = fifteen_applet.desktop

