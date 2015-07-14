window_picker_libdir=${pkglibdir}/${LIBPANEL_APPLET_API_VERSION}
window_picker_lib_LTLIBRARIES=libwindow-picker-applet.la

wpe-resources.h: wp-resources.gresource.xml
	$(GLIB_COMPILE_RESOURCES) --generate $< --target=$@
wp-resources.c: wp-resources.gresource.xml
	$(GLIB_COMPILE_RESOURCES) --generate $< --target=$@

wp-resources.c wp-resources.h: \
	wp-resources.gresource.xml \
		$(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies wp-resources.gresource.xml)

AM_CFLAGS=\
	$(GNOME_APPLETS_CFLAGS)     \
	$(LIBWNCK_CFLAGS)           \
	$(GCC_FLAGS) 				\
	-I$(top_builddir)			\
	-I$(top_srcdir)				\
	-D_GNU_SOURCE 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE=1 \
	-DGTK_DISABLE_SINGLE_INCLUDES \
    -lm

libwindow_picker_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBWNCK_LIBS)

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
	task-title.c \
	task-title.h \
	wp-resources.c \
	wp-resources.h

EXTRA_DIST = \
	wp-about-logo.png \
	wp-menu.xml \
	wp-resources.gresource.xml

CLEANFILES = \
	wp-resources.c \
	wp-resources.h

-include $(top_srcdir)/git.mk

