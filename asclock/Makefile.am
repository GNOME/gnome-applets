INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = asclock_applet

asclock_applet_SOURCES = \
	asclock.c	\
	asclock.h	\
	draw.c          \
        timezone.c      \
        dialogs.c


asclock_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	asclock_applet.desktop	\
	clk.xpm			\
	led.xpm			\
	weekday.xpm             \
        month.xpm               \
        date.xpm		\
	asclock_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = asclock_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = asclock_applet.gnorba
