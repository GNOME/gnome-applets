SUBDIRS = docs

INCLUDES = -I. -I$(srcdir) 				\
	$(GNOME_APPLETS_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) 

CFLAGS = -ansi -Werror -Wmissing-prototypes -Wmissing-declarations

bin_PROGRAMS = charpick_applet2

charpick_applet2_SOURCES = charpick.c \
                          charpick.h \
                          properties.c

charpick_applet2_LDADD = \
		       $(GNOME_APPLETS_LIBS)

charpickbitmapsdir = $(datadir)/pixmaps/
charpickbitmaps_DATA = charpick.png

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_CharpickerApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

EXTRA_DIST = \
	charpick.png $(server_in_files)	
	

