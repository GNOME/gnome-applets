INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = fresh_applet

fresh_applet_SOURCES = freshapp.c

fresh_applet_LDADD = \
		     $(GNOME_LIBDIR) -lXt \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	fresh_applet.desktop	\
	fresh_applet.gnorba

sysdir = $(datadir)/applets/Amusements
sys_DATA = fresh_applet.desktop

gnorbadir = $(sysconfdir)/CORBA/servers
gnorba_DATA = fresh_applet.gnorba
