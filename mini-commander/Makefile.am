INCLUDES = 						\
	-I.						\
	-I$(srcdir)					\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(top_builddir)/panel	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = mini_commander_applet

mini_commander_applet_SOURCES =	\
	mini-commander_applet.c	\
	preferences.c		\
	exec.c			\
	message.c		\
	command_line.c		\
	cmd_completion.c	\
	about.c

mini_commander_applet_LDADD =		\
	../../panel/libpanel_applet.la	\
	$(GNOME_LIBDIR)			\
	$(GNOMEGNORBA_LIBS)		\
	$(INTLLIBS) 

EXTRA_DIST = mini-commander_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = mini-commander_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = mini-commander_applet.gnorba
