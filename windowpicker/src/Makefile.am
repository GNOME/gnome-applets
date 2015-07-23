NULL =

window_picker_libdir=${pkglibdir}/${LIBPANEL_APPLET_API_VERSION}
window_picker_lib_LTLIBRARIES=libwindow-picker-applet.la

libwindow_picker_applet_la_CFLAGS = \
	-D_GNU_SOURCE \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE=1 \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(GCC_FLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libwindow_picker_applet_la_LDFLAGS = \
	-module -avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libwindow_picker_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBWNCK_LIBS) \
	$(LIBM) \
	$(NULL)

libwindow_picker_applet_la_SOURCES = \
	wp-about-dialog.c \
	wp-about-dialog.h \
	wp-applet.c \
	wp-applet.h \
	wp-preferences-dialog.c \
	wp-preferences-dialog.h \
	task-item.c \
	task-item.h \
	task-list.c \
	task-list.h \
	wp-resources.c \
	wp-resources.h \
	wp-task-title.c \
	wp-task-title.h \
	$(NULL)

wp-resources.c: wp-resources.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/wp-resources.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate --c-name wp $<

wp-resources.h: wp-resources.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/wp-resources.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate --c-name wp $<

EXTRA_DIST = \
	wp-about-logo.png \
	wp-menu.xml \
	wp-preferences-dialog.ui \
	wp-resources.gresource.xml \
	$(NULL)

CLEANFILES = \
	wp-resources.c \
	wp-resources.h \
	$(NULL)

-include $(top_srcdir)/git.mk
