NULL =

uidir	= $(pkgdatadir)/ui

modem_lights_appletlibdir = $(pkglibdir)
modem_lights_appletlib_LTLIBRARIES = libmodem-lights-applet.la

libmodem_lights_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.modem-lights\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-DMODEM_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	$(NULL)

libmodem_lights_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(XML2_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libmodem_lights_applet_la_SOURCES = \
	modem-applet.c \
	modem-applet.h \
	$(NULL)

libmodem_lights_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libmodem_lights_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(MODEMLIGHTS_LIBS) \
	$(XML2_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
