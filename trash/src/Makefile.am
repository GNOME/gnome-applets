NULL =

trashapplet_CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
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
	trash-resources.c \
	trash-resources.h \
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

trash-resources.c: trash-resources.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/trash-resources.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate --c-name trash $<

trash-resources.h: trash-resources.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/trash-resources.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate --c-name trash $<

EXTRA_DIST = \
	trash-empty.ui \
	trash-menu.xml \
	trash-resources.gresource.xml \
	$(NULL)

CLEAN_FILES = \
	trash-resources.c \
	trash-resources.h \
	$(NULL)

-include $(top_srcdir)/git.mk
