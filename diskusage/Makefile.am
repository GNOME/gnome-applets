SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = diskusage_applet

diskusage_applet_SOURCES = \
	diskusage.c		\
	diskusage.h		\
	properties.c		\
	properties.h

diskusage_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

sysdir = $(datadir)/applets/Monitors
sys_predata = diskusage_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = diskusage_applet.gnorba

man_MANS = diskusage_applet.1

EXTRA_DIST = $(gnorba_DATA)	\
	$(sys_DATA) 		\
	$(sys_predata)		\
	$(man_MANS)


