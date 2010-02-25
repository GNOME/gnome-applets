INCLUDES = -I$(top_srcdir)		\
	   $(GNOME_APPLETS3_CFLAGS)	\
	   $(GIO_CFLAGS)		\
           -DTRASH_MENU_UI_DIR=\""$(datadir)/gnome-2.0/ui"\"

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trashapplet.c \
	trash-empty.h \
	trash-empty.c \
	xstuff.c \
	xstuff.h

trashapplet_LDADD = 		\
	$(GNOME_APPLETS3_LIBS) 	\
	$(GIO_LIBS)

-include $(top_srcdir)/git.mk
