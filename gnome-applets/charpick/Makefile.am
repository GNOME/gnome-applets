NULL =

noinst_LTLIBRARIES = libcharacter-picker-applet.la

libcharacter_picker_applet_la_CPPFLAGS = \
	-DGDK_DISABLE_DEPRECATION_WARNINGS \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.character-picker\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libcharacter_picker_applet_la_CFLAGS = \
	$(GNOME_PANEL_CFLAGS) \
	$(GUCHARMAP_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libcharacter_picker_applet_la_SOURCES = \
	charpick-applet.c \
	charpick-applet.h \
	charpick.h \
	properties.c \
	$(NULL)

libcharacter_picker_applet_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libcharacter_picker_applet_la_LIBADD = \
	$(GNOME_PANEL_LIBS) \
	$(GUCHARMAP_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
