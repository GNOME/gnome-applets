SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = webcontrol_applet

webcontrol_applet_SOURCES = \
	properties.c		\
	properties.h		\
	session.c		\
	session.h		\
	webcontrol.c		\
	webcontrol.h

webcontrol_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Network
sys_predata = webcontrol_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = webcontrol_applet.gnorba

man_MANS = webcontrol_applet.1

EXTRA_DIST = \
	$(gnorba_DATA) $(sys_DATA) $(sys_predata) \
	$(man_MANS)
	
