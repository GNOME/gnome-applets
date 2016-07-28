NULL =

trashapplet_CPPFLAGS = \
	-DTRASH_MENU_UI_DIR=\""$(pkgdatadir)/ui"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-I$(top_srcdir) \
	$(GNOME_APPLETS_CFLAGS)	\
	$(GIO_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libexec_PROGRAMS = trashapplet

trashapplet_SOURCES = \
	trash-applet.c \
	trash-applet.h \
	trash-empty.h \
	trash-empty.c \
	$(NULL)

trashapplet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

trashapplet_LDADD = 		\
	$(GNOME_APPLETS_LIBS) 	\
	$(GIO_LIBS)		\
	$(X_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
