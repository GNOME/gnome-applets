
INCLUDES =  -I$(srcdir)  -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
		 -I$(includedir)  $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gtik2_applet

gtik2_applet_SOURCES = gtik.c 

gtik2_applet_LDADD = 	$(GNOME_LIBDIR) \
			$(GNOMEUI_LIBS) \
			$(GNOME_APPLETS_LIBS) \
			$(GHTTP_LIB)

EXTRA_DIST = \
	gtik2_applet.desktop gtik2_applet.gnorba README AUTHORS ChangeLog

sysdir = $(datadir)/applets/Network
sys_DATA = gtik2_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gtik2_applet.gnorba
