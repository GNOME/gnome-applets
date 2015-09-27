NULL =

trashapplet_CPPFLAGS = \
	-DTRASH_MENU_UI_DIR=\""$(pkgdatadir)/ui"\" \
	-I$(top_srcdir) \
	$(GNOME_APPLETS_CFLAGS)	\
	$(GIO_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trashapplet.c \
	trash-empty.h \
	trash-empty.c

trashapplet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

trashapplet_LDADD = 		\
	$(GNOME_APPLETS_LIBS) 	\
	$(GIO_LIBS)		\
	$(X_LIBS)

-include $(top_srcdir)/git.mk
