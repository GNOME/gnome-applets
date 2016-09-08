NULL =

command_applet_libdir = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)
command_applet_lib_LTLIBRARIES = libcommand-applet.la

libcommand_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	$(NULL)

libcommand_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libcommand_applet_la_SOURCES = \
	command.c \
	$(NULL)

libcommand_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libcommand_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
