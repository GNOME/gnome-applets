SUBDIRS = docs pixmaps

INCLUDES = \
	-I. \
	-I$(srcdir) \
	-I$(top_srcdir)/intl \
	-I$(top_builddir)/intl \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir) \
	$(PIXBUF_CFLAGS) \
	$(CANVAS_PIXBUF_CFLAGS) \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = jbc_applet

jbc_applet_SOURCES = \
	jbc-applet.c		\
	jbc-applet.h

jbc_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) \
	$(PIXBUF_LIBS) \
	$(CANVAS_PIXBUF_LIBS) \
	$(INTLLIBS)

sysdir = $(datadir)/applets/Clocks
sys_predata = jbc_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-jbc.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = jbc_applet.gnorba

man_MANS = jbc_applet.1

EXTRA_DIST = README AUTHORS ChangeLog gnome-jbc.png	\
	$(gnorba_DATA) $(sys_DATA) $(sys_predata)	\
	$(man_MANS)


# dist-hook:
#	mkdir $(distdir)/pixmaps
#	cp $(srcdir)/pixmaps/*.xpm $(distdir)/pixmaps
