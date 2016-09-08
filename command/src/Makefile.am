NULL =

libexec_PROGRAMS = command-applet

command_applet_CFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

command_applet_SOURCES = \
	command.c \
	$(NULL)

command_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

command_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
