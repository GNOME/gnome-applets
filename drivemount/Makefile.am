SUBDIRS = help

INCLUDES = \
-I. -I$(srcdir) \
-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
-I$(top_srcdir)/screen-exec \
$(GNOME_APPLETS_CFLAGS) 

libexec_PROGRAMS = drivemount_applet2

desktop_iconsdir = $(datadir)/pixmaps
desktop_icons_DATA = drivemount-applet.png

drivemount_applet2_SOURCES = \
drivemount.c \
drivemount.h \
properties.c \
properties.h

drivemount_applet2_LDADD =				\
	$(top_builddir)/screen-exec/libscreen-exec.la	\
	$(GNOME_APPLETS_LIBS) 

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_DriveMountApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_DriveMountApplet.xml

@INTLTOOL_SERVER_RULE@

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = drivemount.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

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
usbstick_h_in.xpm \
usbstick_h_out.xpm \
usbstick_v_in.xpm \
usbstick_v_out.xpm \
GNOME_DriveMountApplet.server.in.in \
$(schemas_in_files)	\
$(ui_DATA)	\
drivemount-applet.png

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
