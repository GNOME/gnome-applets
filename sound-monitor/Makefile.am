INCLUDES = -I$(srcdir) -I$(builddir) \
	-I$(top_srcdir) -I$(top_builddir)	\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = sound-monitor_applet

sound_monitor_applet_SOURCES = \
	main.c		\
	theme.c		\
	esdcalls.c	\
	properties.c	\
	manager.c	\
	sound-monitor.h

sound_monitor_applet_LDADD = \
	$(GNOME_LIBDIR) 	\
	$(GNOME_APPLETS_LIBS) $(INTLLIBS)

EXTRA_DIST = sound-monitor_applet.desktop sound-monitor_applet.gnorba

sysdir = $(datadir)/applets/Multimedia
sys_DATA = sound-monitor_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = sound-monitor_applet.gnorba

SUBDIRS = def_theme themes pvd
