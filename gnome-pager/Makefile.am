INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = gnomepager_applet

gnomepager_applet_SOURCES = \
	gnomepager_applet.c \
	gnomepager.h

gnomepager_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) \
		     $(GNOMEGNORBA_LIBS) $(INTLLIBS) 

EXTRA_DIST = gnomepager_applet.desktop gnomepager_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = gnomepager_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gnomepager_applet.gnorba 
