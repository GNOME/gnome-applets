NULL =

noinst_LTLIBRARIES = libcommand-applet.la

libcommand_applet_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.command\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libcommand_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libcommand_applet_la_SOURCES = \
	command-applet.c \
	command-applet.h \
	ga-command.c \
	ga-command.h \
	$(NULL)

libcommand_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libcommand_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
