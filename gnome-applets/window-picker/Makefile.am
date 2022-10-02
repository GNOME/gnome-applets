NULL =

noinst_LTLIBRARIES=libwindow-picker-applet.la

libwindow_picker_applet_la_CFLAGS = \
	-D_GNU_SOURCE \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.window-picker\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE=1 \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	$(GNOME_PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(X11_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libwindow_picker_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libwindow_picker_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(LIBWNCK_LIBS) \
	$(X11_LIBS) \
	$(LIBM) \
	$(NULL)

libwindow_picker_applet_la_SOURCES = \
	wp-applet-private.h \
	wp-applet.c \
	wp-applet.h \
	wp-preferences-dialog.c \
	wp-preferences-dialog.h \
	task-item.c \
	task-item.h \
	task-list.c \
	task-list.h \
	wp-task-title.c \
	wp-task-title.h \
	$(NULL)

-include $(top_srcdir)/git.mk
