SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	$(LIBGTOP_INCS)

bin_PROGRAMS = cpumemusage_applet

cpumemusage_applet_SOURCES = \
	cpumemusage.c	\
	proc.c		\
	proc.h

cpumemusage_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

sysdir = $(datadir)/applets/Monitors
sys_predata = cpumemusage_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-cpu-mem.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = cpumemusage_applet.gnorba 

man_MANS = cpumemusage_applet.1

EXTRA_DIST = \
	$(pixmap_DATA)	\
	$(gnorba_DATA)	\
	$(sys_DATA)	\
	$(sys_predata)	\
	$(man_MANS)

