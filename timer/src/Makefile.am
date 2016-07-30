NULL =

libexec_PROGRAMS = timer-applet

timer_applet_CFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

timer_applet_SOURCES = \
	timer-applet.c \
	$(NULL)

timer_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

timer_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
