INCLUDES =	 			\
	-I..		 		\
	-I../../..	 		\
	-I$(srcdir)/../../..	 	\
	-I$(includedir)

bin_PROGRAMS = cdplayer_applet

cdplayer_applet_SOURCES = \
        cdplayer.c         \
	led.c \
	cdrom-$(CDROM_HOST).c \
        ../applet-lib.cc   \
        ../applet-lib.h    \
        ../applet-widget.c \
        ../applet-widget.h \
        ../$(CORBA_SOURCES)

MICO_VERS = 2.0.3
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

cdplayer_applet_LDADD = $(GNOME_LIBDIR) $(MICO_LIBS) -lXt $(GNOMEUI_LIBS) $(INTLLIBS)

EXTRA_DIST = cdrom-linux.c cdrom-solaris.c

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images
