INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = bussign_applet

bussign_applet_SOURCES = \
	bussign.c	\
	bussign.h


bussign_applet_LDADD = \
		     $(GNOME_LIBDIR)	\
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)      \
                     $(GHTTP_LIB)

EXTRA_DIST = bussign_applet.desktop bussign_applet.gnorba

sysdir = $(datadir)/applets/Amusements
sys_DATA = bussign_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = bussign_applet.gnorba
