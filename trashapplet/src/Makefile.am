INCLUDES = -I$(top_srcdir)		\
	   $(GNOME_APPLETS_CFLAGS)	\
	   $(GIO_CFLAGS) 		\
           $(LIBGLADE_CFLAGS)

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trashapplet.c \
	trashapplet.h \
	trash-monitor.h \
	trash-monitor.c \
	xstuff.c \
	xstuff.h

trashapplet_LDADD = 		\
	$(GNOME_APPLETS_LIBS) 	\
	$(GIO_LIBS) 		\
	$(LIBGLADE_LIBS)
