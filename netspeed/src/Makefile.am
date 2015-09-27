NULL =

uidir = $(pkgdatadir)/ui

netspeed_applet2_CFLAGS = \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DNETSPEED_MENU_UI_DIR=\""$(uidir)"\" \
	-I$(top_srcdir) \
	-I$(includedir) \
	$(GNOME_APPLETS_CFLAGS) \
	$(GTOP_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libexec_PROGRAMS = netspeed_applet2

netspeed_applet2_SOURCES = \
  backend.h \
  backend.c \
  label.h \
  label.c \
  netspeed.h \
  netspeed.c \
  preferences.h \
  preferences.c

netspeed_applet2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

netspeed_applet2_LDADD = \
        $(GNOME_APPLETS_LIBS) \
        $(GTOP_APPLETS_LIBS) \
        $(IWLIB)

-include $(top_srcdir)/git.mk
