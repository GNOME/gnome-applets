INCLUDES =	 			\
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

cdplayer_applet_LDADD = $(GNOME_LIBDIR) $(MICO_LIBS) -lXt $(GNOMEUI_LIBS) $(INTLLIBS)

EXTRA_DIST = cdrom-linux.c cdrom-solaris.c

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images
