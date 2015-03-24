uidir = $(pkgdatadir)/ui

AM_CFLAGS = \
       -I$(top_srcdir) \
       -I$(includedir) \
       $(GNOME_APPLETS_CFLAGS) \
       $(GTOP_APPLETS_CFLAGS) \
       -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
       -DNETSPEED_MENU_UI_DIR=\""$(uidir)"\" \
       -DDATADIR=\""$(datadir)/"\" \
       -DPREFIX=\""$(prefix)/"\" \
       -DLIBDIR=\""$(libdir)/"\" \
       -DSYSCONFDIR=\""$(sysconfdir)/"\"

libexec_PROGRAMS = netspeed_applet2

netspeed_applet2_SOURCES = backend.h backend.c netspeed.c

netspeed_applet2_LDADD = \
        $(GNOME_APPLETS_LIBS) \
        $(GTOP_APPLETS_LIBS) \
        $(IWLIB)

-include $(top_srcdir)/git.mk
