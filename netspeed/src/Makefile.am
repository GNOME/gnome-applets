INCLUDES = -I$(top_srcdir) -I$(includedir) $(NETSPEED_CFLAGS) $(NETSPEED_LIBGTOP_CFLAGS) \
       -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
       -DDATADIR=\""$(datadir)/"\" \
       -DPREFIX=\""$(prefix)/"\" \
       -DLIBDIR=\""$(libdir)/"\" \
       -DSYSCONFDIR=\""$(sysconfdir)/"\"

libexec_PROGRAMS = netspeed_applet2

netspeed_applet2_SOURCES = backend.h backend.c netspeed.c

netspeed_applet2_LDADD = $(NETSPEED_LIBS) $(NETSPEED_LIBGTOP_LIBS) $(IWLIB) $(INTLLIBS)
