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

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = cdplayer_applet.gnorba 

sysdir = $(datadir)/applets/Multimedia
sys_predata = cdplayer_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

man_MANS = cdplayer_applet.1

EXTRA_DIST = cdrom-linux.c cdrom-solaris.c cdrom-irix.c	\
	     $(gnorba_DATA) $(sys_DATA)		\
	     $(sys_predata) $(man_MANS)
