NULL =

libexec_PROGRAMS = charpick_applet2

charpick_applet2_CFLAGS = \
	-I. \
	-I$(srcdir) \
	-DCHARPICK_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(GUCHARMAP_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

charpick_applet2_SOURCES = \
	charpick.c \
	charpick.h \
	properties.c \
	$(NULL)

charpick_applet2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

charpick_applet2_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(GUCHARMAP_LIBS) \
	$(NULL)
