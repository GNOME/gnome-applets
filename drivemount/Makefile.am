
INCLUDES = \
-I. -I$(srcdir) \
$(GNOME_APPLETS_CFLAGS) \
-DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
-I$(includedir) 

bin_PROGRAMS = drivemount_applet2

drivemount_applet2_SOURCES = \
drivemount.c \
drivemount.h \
properties.c \
properties.h

drivemount_applet2_LDADD = $(GNOME_APPLETS_LIBS)  

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_DriveMountApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

EXTRA_DIST = \
cdrom_h_in.xpm \
cdrom_h_out.xpm \
cdrom_v_in.xpm \
cdrom_v_out.xpm \
floppy_h_in.xpm	\
floppy_h_out.xpm \
floppy_v_in.xpm \
floppy_v_out.xpm \
harddisk_h_in.xpm \
harddisk_h_out.xpm \
harddisk_v_in.xpm \
harddisk_v_out.xpm \
zipdrive_h_in.xpm \
zipdrive_h_out.xpm \
zipdrive_v_in.xpm \
zipdrive_v_out.xpm \
jazdrive_h_in.xpm \
jazdrive_h_out.xpm \
jazdrive_v_in.xpm \
jazdrive_v_out.xpm \
$(server_in_files)
