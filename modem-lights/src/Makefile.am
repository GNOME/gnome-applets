NULL =

uidir	= $(pkgdatadir)/ui

libexec_PROGRAMS = modem_applet

modem_applet_CFLAGS = \
	-I. \
	-I$(srcdir) \
	-DMODEM_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(XML2_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

modem_applet_SOURCES = \
	modem-applet.c \
	modem-applet.h \
	$(NULL)

modem_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

modem_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(MODEMLIGHTS_LIBS) \
	$(XML2_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
