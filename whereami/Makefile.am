SUBDIRS = docs

INCLUDES = -DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = whereami_applet

whereami_applet_SOURCES = whereami.c

whereami_applet_LDADD = $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

sysdir = $(datadir)/applets/Utility
sys_predata = whereami_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = whereami_applet.gnorba

EXTRA_DIST = AUTHORS README TODO	\
	$(gnorba_DATA) $(sys_DATA) $(sys_predata)
