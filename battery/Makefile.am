INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = battery_applet

battery_applet_SOURCES = battery.c 	\
			 battery.h	\
			 read-battery.c	\
			 read-battery.h	\
			 properties.c	\
			 properties.h	\
			 session.c	\
			 session.h

battery_applet_LDADD = ../../panel/libpanel_applet.la \
			$(GNOME_LIBDIR)  \
			$(GNOMEGNORBA_LIBS) $(INTLLIBS)

EXTRA_DIST = battery_applet.desktop battery_applet.gnorba

sysdir = $(datadir)/applets/Monitors
sys_DATA = battery_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = battery_applet.gnorba
