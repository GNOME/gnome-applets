INCLUDES = -I$(top_srcdir)		\
	   $(GNOME_APPLETS_CFLAGS)	\
	   $(GIO_CFLAGS)		\
	   $(WARN_CFLAGS)		\
           -DTRASH_MENU_UI_DIR=\""$(pkgdatadir)/ui"\"

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trashapplet.c \
	trash-empty.h \
	trash-empty.c \
	xstuff.c \
	xstuff.h

trashapplet_LDADD = 		\
	$(GNOME_APPLETS_LIBS) 	\
	$(GIO_LIBS)		\
	$(X_LIBS)

-include $(top_srcdir)/git.mk
