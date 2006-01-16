INCLUDES = -I$(top_srcdir)		\
	   $(GNOME_APPLETS_CFLAGS)	\
	   $(GNOME_VFS_APPLETS_CFLAGS) \
           $(LIBGLADE_CFLAGS)

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trashapplet.c \
	trashapplet.h \
	trash-monitor.h \
	trash-monitor.c \
	xstuff.c \
	xstuff.h

trashapplet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GNOME_VFS_APPLETS_LIBS) \
	$(LIBGLADE_LIBS)
