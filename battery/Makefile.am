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

bin_PROGRAMS = battery_applet

battery_applet_SOURCES = battery.c 	\
			 battery.h	\
			 read-battery.c	\
			 read-battery.h	\
			 properties.c	\
			 properties.h	\
			 session.c	\
			 session.h

battery_applet_LDADD = ../../panel/libpanel_applet.la \
			$(GNOME_LIBDIR) $(ORB_LIBS)      \
			$(GNOMEUI_LIBS) $(INTLLIBS)

battery_applet_LINK = $(CXXLINK)

EXTRA_DIST = battery_applet.dekstop

sysdir = $(datadir)/applets/Monitors
sys_DATA = battery_applet.desktop
