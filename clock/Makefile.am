INCLUDES =	 			\
	-I../libapplet	 		\
	-I$(srcdir)/../libapplet	\
	-I../../..	 		\
	-I$(srcdir)/../../..	 	\
	-I$(includedir)

bin_PROGRAMS = clock_applet

clock_applet_SOURCES = clock.c

MICO_VERS = 2.0.3
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

clock_applet_LDADD = ../libapplet/libapplet.a \
		     $(GNOME_LIBDIR) $(MICO_LIBS) -lXt \
		     $(GNOMEUI_LIBS) $(INTLLIBS) -lstdc++

