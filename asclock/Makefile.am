INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = asclock_applet

asclock_applet_SOURCES = \
	asclock.c	\
	asclock.h	\
	draw.c          \
        timezone.c      \
        dialogs.c


asclock_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	asclock_applet.desktop	\
	clk.xpm			\
	led.xpm			\
	weekday.xpm             \
        month.xpm               \
        date.xpm

sysdir = $(datadir)/applets/Utility
sys_DATA = asclock_applet.desktop

