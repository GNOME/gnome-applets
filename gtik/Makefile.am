SUBDIRS = macros

INCLUDES = $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gtik_applet

gtik_applet_SOURCES = gtik.c 

gtik_applet_LDADD = 	$(GNOME_LIBDIR) \
			$(GNOMEUI_LIBS) \
			$(GNOME_APPLETS_LIBS) \
			$(GHTTP_LIB)

EXTRA_DIST = \
	gtik_applet.desktop gtik_applet.gnorba

sysdir = $(datadir)/applets/Amusements
sys_DATA = gtik_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = gtik_applet.gnorba
