INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = drivemount_applet

drivemount_applet_SOURCES = \
	drivemount.c	\
	drivemount.h	\
	properties.c

drivemount_applet_LDADD = \
		     $(GNOME_LIBDIR) \
		     $(GNOME_APPLETS_LIBS) $(INTLLIBS) 

EXTRA_DIST = \
	drivemount_applet.desktop	\
	cdrom_h_in.xpm			\
	cdrom_h_out.xpm			\
	cdrom_v_in.xpm			\
	cdrom_v_out.xpm			\
	floppy_h_in.xpm			\
	floppy_h_out.xpm		\
	floppy_v_in.xpm			\
	floppy_v_out.xpm		\
	harddisk_h_in.xpm		\
	harddisk_h_out.xpm		\
	harddisk_v_in.xpm		\
	harddisk_v_out.xpm		\
	zipdrive_h_in.xpm		\
	zipdrive_h_out.xpm		\
	zipdrive_v_in.xpm		\
	zipdrive_v_out.xpm		\
	jazdrive_h_in.xpm		\
	jazdrive_h_out.xpm		\
	jazdrive_v_in.xpm		\
	jazdrive_v_out.xpm		\
	drivemount_applet.gnorba

sysdir = $(datadir)/applets/Utility
sys_DATA = drivemount_applet.desktop

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = drivemount_applet.gnorba
