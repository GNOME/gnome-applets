
INCLUDES =  -I$(srcdir)  -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
		 -I$(includedir)  $(GNOME_INCLUDEDIR) $(VFS_CFLAGS) \
		 -DGTIK_APPLET_NAME=\"gtik2_applet\"

bin_PROGRAMS = gtik2_applet

gtik2_applet_SOURCES = gtik.c 

gtik2_applet_LDADD = 	$(GNOME_LIBDIR) \
			$(GNOMEUI_LIBS) \
			$(GNOME_APPLETS_LIBS) \
			$(VFS_LIBS)

sysdir = $(datadir)/applets/Network
sys_predata = gtik2_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gtik2_applet.gnorba

man_MANS = gtik2_applet.1

EXTRA_DIST = \
	README AUTHORS ChangeLog	\
	$(gnorba_DATA) $(sys_DATA)	\
	$(sys_predata)			\
	$(man_MANS)


