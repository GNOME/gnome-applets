INCLUDES =	 			\
	-I../libapplet	 		\
	-I$(srcdir)/../libapplet	\
	-I../../..	 		\
	-I$(srcdir)/../../..	 	\
	-I$(includedir)

bin_PROGRAMS = cdplayer_applet

cdplayer_applet_SOURCES = \
        cdplayer.c         \
	led.c \
	cdrom-$(CDROM_HOST).c 

MICO_VERS = 2.0.3
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

cdplayer_applet_LDADD = ../libapplet/libapplet.a \
			$(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		        $(GNOMEUI_LIBS) $(INTLLIBS) -lstdc++

EXTRA_DIST = cdrom-linux.c cdrom-solaris.c

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images
