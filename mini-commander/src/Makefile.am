## Process this file with automake to produce Makefile.in

INCLUDES = 						\
	-I.						\
	-I$(srcdir)					\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = mini_commander_applet

mini_commander_applet_SOURCES =			\
	about.c					\
	about.h					\
	cmd_completion.c			\
	cmd_completion.h			\
	command_line.c				\
	command_line.h				\
	exec.c					\
	exec.h					\
	message.c				\
	message.h				\
	mini-commander_applet.c			\
	mini-commander_applet.h			\
	help.c					\
	help.h					\
	history.c				\
	history.h				\
	macro.c					\
	macro.h					\
	preferences.c				\
	preferences.h				\
	terminal.c				\
	terminal.h

mini_commander_applet_LDADD =				\
	$(GNOME_LIBDIR)					\
	$(GNOME_APPLETS_LIBS)				\
	$(ZVT_LIBS)					\
	$(INTLLIBS) 

# currently the installation of these bitmaps wouldn't be necessary
bitmapsdir = $(datadir)/pixmaps/mini-commander
bitmaps_DATA = browser-mini.xpm history-mini.xpm

sysdir = $(datadir)/applets/Utility
sys_predata = mini-commander_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = mini-commander_applet.gnorba

man_MANS = mini-commander_applet.1

EXTRA_DIST = $(gnorba_DATA)	\
	$(sys_DATA)		\
	$(sys_predata)		\
	$(bitmaps_DATA)		\
	$(man_MANS)

