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

bin_PROGRAMS = bussign_applet

bussign_applet_SOURCES = \
	bussign.c	\
	bussign.h	\
	http_get.c	\
	http_get.h


bussign_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)

bussign_applet_LINK = $(CXXLINK)

EXTRA_DIST = bussign_applet.desktop

sysdir = $(datadir)/applets/Amusements
sys_DATA = bussign_applet.desktop
