INCLUDES =	 			\
	-I..		 		\
	-I../../..	 		\
	-I$(srcdir)/../../..	 	\
	-I$(includedir)

bin_PROGRAMS = clock_applet

clock_applet_SOURCES = \
        clock.c         \
        ../applet-lib.cc   \
        ../applet-lib.h    \
        ../applet-widget.c \
        ../applet-widget.h \
        ../$(CORBA_SOURCES)

MICO_VERS = 2.0.3
MICO_LIBS = -lmicocoss$(MICO_VERS) -lmicoaux$(MICO_VERS) -lmico$(MICO_VERS)

clock_applet_LDADD = $(GNOME_LIBDIR) $(MICO_LIBS) -lXt $(GNOMEUI_LIBS) $(INTLLIBS)

