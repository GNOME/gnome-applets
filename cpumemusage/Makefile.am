INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-I$(LIBGTOP_INCS)

bin_PROGRAMS = cpumemusage_applet

cpumemusage_applet_SOURCES = \
	cpumemusage.c	\
	proc.c		\
	proc.h


cpumemusage_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST = cpumemusage_applet.desktop cpumemusage_applet.gnorba

sysdir = $(datadir)/applets/Monitors
sys_DATA = cpumemusage_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = cpumemusage_applet.gnorba 
