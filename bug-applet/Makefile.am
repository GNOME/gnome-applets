XML_CFLAGS = `xml-config --cflags`

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) 		\
	$(XML_CFLAGS)

bin_PROGRAMS = bug-applet

bug_applet_SOURCES = \
	bug-applet.c

bug_applet_LDADD = \
	$(GNOME_LIBDIR)		\
	$(GNOME_APPLETS_LIBS)	\
	$(INTLLIBS)      	\
	$(GHTTP_LIB) $(GNOME_XML_LIB)

typesdir = $(datadir)/bug-applet
types_DATA = bugtypes.xml

sysdir = $(datadir)/applets/Utility
sys_DATA = bug-applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = bug-applet.gnorba

EXTRA_DIST = \
	$(types_DATA)	\
	$(sys_DATA)	\
	$(gnorba_DATA)
