NULL =

uidir = $(pkgdatadir)/ui

charpick_applet_libdir = $(pkglibdir)
charpick_applet_lib_LTLIBRARIES = libcharacter-picker-applet.la

libcharacter_picker_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-DCHARPICK_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.character-picker\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libcharacter_picker_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(GUCHARMAP_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libcharacter_picker_applet_la_SOURCES = \
	charpick.c \
	charpick.h \
	properties.c \
	$(NULL)

libcharacter_picker_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libcharacter_picker_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GUCHARMAP_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
