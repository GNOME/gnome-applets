NULL =

command_applet_libdir = $(pkglibdir)
command_applet_lib_LTLIBRARIES = libcommand-applet.la

libcommand_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.command\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libcommand_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libcommand_applet_la_SOURCES = \
	command.c \
	ga-command.h \
	ga-command.c \
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
