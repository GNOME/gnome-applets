INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 

bin_PROGRAMS = hal_applet 

hal_applet_SOURCES = hal.c

hal_applet_LDADD = \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

halbitmapsdir = $(datadir)/pixmaps/hal
halbitmaps_DATA = hal.png

sysdir = $(datadir)/applets/Amusements
sys_predata = hal_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = hal_applet.gnorba

EXTRA_DIST = hal.png	\
	$(gnorba_DATA) $(sys_DATA) $(sys_predata)

