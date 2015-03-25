uidir = $(pkgdatadir)/ui

AM_CFLAGS = \
       -I$(top_srcdir) \
       -I$(includedir) \
       $(GNOME_APPLETS_CFLAGS) \
       $(GTOP_APPLETS_CFLAGS) \
       -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
       -DNETSPEED_MENU_UI_DIR=\""$(uidir)"\"

libexec_PROGRAMS = netspeed_applet2

netspeed_applet2_SOURCES = backend.h backend.c netspeed.c netspeed.h

netspeed_applet2_LDADD = \
        $(GNOME_APPLETS_LIBS) \
        $(GTOP_APPLETS_LIBS) \
        $(IWLIB)

-include $(top_srcdir)/git.mk
