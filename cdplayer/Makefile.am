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

bin_PROGRAMS = cdplayer_applet

cdplayer_applet_SOURCES =	\
        cdplayer.c		\
	cdplayer.h		\
	cdrom-interface.h	\
	led.c			\
	led.h			\
	cdrom-$(CDROM_HOST).c 


cdplayer_applet_LDADD = ../../panel/libpanel_applet.la		\
		     $(GNOME_LIBDIR) $(MICO_LIBS)     		\
		     $(GNOMEUI_LIBS) $(INTLLIBS)

cdplayer_applet_LINK = $(CXXLINK)

EXTRA_DIST = cdplayer_applet.desktop cdrom-linux.c cdrom-solaris.c

sysdir = $(datadir)/applets/Multimedia
sys_DATA = cdplayer_applet.desktop

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images
