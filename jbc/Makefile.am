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

bin_PROGRAMS = jbc_applet

jbc_applet_SOURCES = jbc.c

jbc_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

jbc_applet_LINK = $(CXXLINK)

EXTRA_DIST = \
	jbc_applet.desktop	\

sysdir = $(datadir)/applets/Amusements
sys_DATA = jbc_applet.desktop
