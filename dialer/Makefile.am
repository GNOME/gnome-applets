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

bin_PROGRAMS = dialer_applet

dialer_applet_SOURCES =	\
        dialer.c	\
	dialer.h


dialer_applet_LDADD = ../../panel/libpanel_applet.la		\
		     $(GNOME_LIBDIR) $(MICO_LIBS)     		\
		     $(GNOMEUI_LIBS) $(INTLLIBS)

dialer_applet_LINK = $(CXXLINK)

EXTRA_DIST = dialer_applet.desktop

sysdir = $(datadir)/applets/Network
sys_DATA = dialer_applet.desktop

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images
