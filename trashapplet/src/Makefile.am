INCLUDES = -I$(top_srcdir)						\
	   $(GNOME_APPLETS_CFLAGS) \
	   $(GNOME_VFS_APPLETS_CFLAGS) \
	   -DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trashapplet.c \
	trashapplet.h \
	eel-extension.h \
	eel-extension.c

trashapplet_LDADD = $(GNOME_APPLETS_LIBS) $(GNOME_VFS_APPLETS_LIBS)
