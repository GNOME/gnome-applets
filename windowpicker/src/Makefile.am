libexec_PROGRAMS=window-picker-applet

PKGDATADIR = $(datadir)/window-picker-applet
AM_CFLAGS=\
	$(DEPS_CFLAGS) 				\
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

window_picker_applet_LDADD = \
	$(DEPS_LIBS)

window_picker_applet_SOURCES = \
	applet.c \
	applet.h \
	task-item.c \
	task-item.h \
	task-list.c \
	task-list.h \
	task-title.c \
	task-title.h 
