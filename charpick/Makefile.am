SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

CFLAGS = -ansi -Werror -Wmissing-prototypes -Wmissing-declarations

bin_PROGRAMS = charpick_applet

charpick_applet_SOURCES = charpick.c \
                          charpick.h \
                          properties.c

charpick_applet_LDADD = \
		       $(GNOME_LIBDIR) \
		       $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

charpickbitmapsdir = $(datadir)/pixmaps/
charpickbitmaps_DATA = charpick.png

sysdir = $(datadir)/applets/Utility
sys_predata = charpick_applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = charpick_applet.gnorba

man_MANS = charpick_applet.1

EXTRA_DIST = \
	charpick.png	\
	$(gnorba_DATA)	\
	$(sys_DATA)	\
	$(sys_predata)	\
	$(man_MANS)


