# trick Automake into defining $(CXXLINK)
if FALSE
BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc
endif

INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = drivemount_applet

drivemount_applet_SOURCES = \
	drivemount.c	\
	drivemount.h	\
	properties.c


drivemount_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(MICO_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

drivemount_applet_LINK = $(CXXLINK)

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
	zipdrive_v_out.xpm

sysdir = $(datadir)/applets/Utility
sys_DATA = drivemount_applet.desktop

