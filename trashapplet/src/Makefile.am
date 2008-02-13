INCLUDES = -I$(top_srcdir)		\
	   $(GNOME_APPLETS_CFLAGS)	\
	   $(GIO_CFLAGS)

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trashapplet.c \
	trash-empty.h \
	trash-empty.c \
	xstuff.c \
	xstuff.h

trashapplet_LDADD = 		\
	$(GNOME_APPLETS_LIBS) 	\
	$(GIO_LIBS)
