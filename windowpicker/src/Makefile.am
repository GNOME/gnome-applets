window_picker_libdir=${pkglibdir}/${LIBPANEL_APPLET_API_VERSION}
window_picker_lib_LTLIBRARIES=libwindow-picker-applet.la

PKGDATADIR = $(datadir)/window-picker-applet
AM_CFLAGS=\
	$(GNOME_APPLETS_CFLAGS)     \
	$(LIBWNCK_CFLAGS)           \
	$(GCC_FLAGS) 				\
	-I$(top_builddir)			\
	-I$(top_srcdir)				\
	-D_GNU_SOURCE 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DPKGDATADIR=\"$(PKGDATADIR)\" \
	-DWINDOW_PICKER_MENU_UI_DIR=\"$(datadir)/$(PACKAGE)/ui\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE=1 \
	-DGTK_DISABLE_SINGLE_INCLUDES \
    -lm

libwindow_picker_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBWNCK_LIBS)

libwindow_picker_applet_la_SOURCES = \
	applet.c \
	applet.h \
	task-item.c \
	task-item.h \
	task-list.c \
	task-list.h \
	task-title.c \
	task-title.h

-include $(top_srcdir)/git.mk

