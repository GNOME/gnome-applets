SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = mixer_applet

mixer_applet_SOURCES = \
	mixer.c		\
	vslider.c	\
	vslider.h	\
	hslider.c	\
	hslider.h	\
	lamp-small.xpm  \
	lamp-small-red.xpm 

mixer_applet_LDADD = \
		     $(MIXER_LIBS) \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Multimedia
sys_predata = mixer_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = mixer_applet.gnorba

man_MANS = mixer_applet.1

EXTRA_DIST = $(gnorba_DATA)	\
	$(sys_DATA)		\
	$(sys_predata)		\
	$(man_MANS)

