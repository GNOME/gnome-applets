
INCLUDES = \
-I. -I$(srcdir) \
-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
$(GNOME_APPLETS_CFLAGS) 

bin_PROGRAMS = drivemount_applet2

desktop_iconsdir = $(datadir)/pixmaps
desktop_icons_DATA = drivemount-applet.png

drivemount_applet2_SOURCES = \
drivemount.c \
drivemount.h \
properties.c \
properties.h

drivemount_applet2_LDADD = $(GNOME_APPLETS_LIBS) 

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_DriveMountApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = drivemount.schemas

EXTRA_DIST = \
cdrom_h_in.xpm \
cdrom_h_out.xpm \
cdrom_v_in.xpm \
cdrom_v_out.xpm \
cdburn_h_in.xpm \
cdburn_h_out.xpm \
cdburn_v_in.xpm \
cdburn_v_out.xpm \
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
$(server_in_files) \
$(schema_DATA)	\
drivemount-applet.png

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
