SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = modemlights_applet

modemlights_applet_SOURCES = \
	modemlights.c	\
	modemlights.h	\
	properties.c

modemlights_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Network
sys_predata = modemlights_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = modemlights_applet.gnorba

man_MANS = modemlights_applet.1

EXTRA_DIST = \
	digits.xpm			\
	$(gnorba_DATA) $(sys_DATA)	\
	$(sys_predata)			\
	$(man_MANS)


