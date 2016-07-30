NULL =

trash_libdir=${pkglibdir}/${LIBPANEL_APPLET_API_VERSION}
trash_lib_LTLIBRARIES=libtrash-applet.la

libtrash_applet_la_CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	$(NULL)

libtrash_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(GIO_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libtrash_applet_la_LDFLAGS = \
	-module -avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libtrash_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GIO_LIBS) \
	$(X_LIBS) \
	$(NULL)

libtrash_applet_la_SOURCES = \
	trash-applet.c \
	trash-applet.h \
	trash-empty.h \
	trash-empty.c \
	trash-resources.c \
	trash-resources.h \
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

CLEANFILES = \
	trash-resources.c \
	trash-resources.h \
	$(NULL)

-include $(top_srcdir)/git.mk
