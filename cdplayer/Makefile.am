SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = cdplayer_applet

cdplayer_applet_SOURCES =	\
        cdplayer.c		\
	cdplayer.h		\
	cdrom-interface.h	\
	led.c			\
	led.h			\
	cdrom-$(CDROM_HOST).c 

cdplayer_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) $(CDROM_LIBS)

EXTRA_DIST = cdplayer_applet.desktop cdrom-linux.c cdrom-solaris.c \
	     cdplayer_applet.gnorba

sysdir = $(datadir)/applets/Multimedia
sys_DATA = cdplayer_applet.desktop

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = cdplayer_applet.gnorba 
