SUBDIRS = def_theme themes pvd docs

INCLUDES = -I$(srcdir) -I$(builddir) \
	-I$(top_srcdir) -I$(top_builddir)	\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-I$(includedir) $(GNOME_INCLUDEDIR)			\
	@PIXBUF_CFLAGS@

CFLAGS += $(ESD_CFLAGS)

bin_PROGRAMS = sound-monitor_applet

sound_monitor_applet_SOURCES = \
	analyzer.c	\
	analyzer.h	\
	def_theme.c	\
	def_theme.h	\
	esdcalls.c	\
	esdcalls.h	\
	fft.c		\
	fft.h		\
	item.c		\
	item.h		\
	main.c		\
	manager.c	\
	manager.h	\
	meter.c		\
	meter.h		\
	pixbuf_ops.c	\
	pixbuf_ops.h	\
	properties.c	\
	scope.c		\
	scope.h		\
	skin.c		\
	skin.h		\
	sound-monitor.h	\
	theme.c		\
	theme.h		\
	typedefs.h	\
	update.c	\
	update.h

sound_monitor_applet_LDADD = \
	$(GNOME_LIBDIR) 	\
	$(GNOME_APPLETS_LIBS) $(INTLLIBS)	\
	@PIXBUF_LIBS@

sysdir = $(datadir)/applets/Multimedia
sys_predata = sound-monitor_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = sound-monitor_applet.gnorba

man_MANS = esdpvd.1 sound-monitor_applet.1

EXTRA_DIST = \
	THEME-SPEC	\
	$(gnorba_DATA)	\
	$(sys_DATA)	\
	$(sys_predata)	\
	$(man_MANS)
	

