SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = screenshooter_applet

screenshooter_applet_SOURCES = \
	screenshooter_applet.c		\
	screenshooter_applet.h		\
	root_icon.xpm  \
	window_icon.xpm 

screenshooter_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

scrshotdir = $(bindir)
scrshot_SCRIPTS = update_scrshot_page

pngdir = $(datadir)/pixmaps
png_DATA = screenshooter_applet.png

sysdir = $(datadir)/applets/Utility
sys_predata = screenshooter_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = screenshooter_applet.gnorba

man_MANS = screenshooter_applet.1

EXTRA_DIST = $(png_DATA)	\
	$(scrshot_SCRIPTS)	\
	$(gnorba_DATA)		\
	$(sys_DATA)		\
	$(sys_predata)		\
	$(man_MANS)
	

