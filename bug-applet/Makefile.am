XML_CFLAGS = `xml-config --cflags`

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 				\
	$(XML_CFLAGS)

bin_PROGRAMS = bug-applet

bug_applet_SOURCES = \
	bug-applet.c

bug_applet_LDADD = \
	$(GNOME_APPLETS_LIBS)	\
	$(INTLLIBS)      	\
	$(GHTTP_LIB) $(GNOME_XML_LIB)

typesdir = $(datadir)/bug-applet
types_DATA = bugtypes.xml

sysdir = $(datadir)/applets/Utility
sys_predata = bug-applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = bug-applet.gnorba

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-bug-tracker.png

EXTRA_DIST = \
	$(types_DATA)	\
	$(pixmaps_DATA)	\
	$(gnorba_DATA) $(sys_DATA) $(sys_predata)
