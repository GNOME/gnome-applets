
gladedir = $(datadir)/battstat_applet
glade_DATA = battstat_applet.glade

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = battstat.png

if HAVE_LIBAPM
APMLIB = -lapm
APMDIR =
APMINC = 
else
APMLIB = apmlib/libapm.a
APMDIR = apmlib
APMINC = -I$(srcdir)/apmlib/
endif

SUBDIRS = docs sounds $(APMDIR)

DIST_SUBDIRS = docs sounds apmlib

INCLUDES = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBGLADE_CFLAGS) \
	$(APMINC) \
	-I$(top_srcdir)/screen-exec \
	-DDATA_DIR=\"$(datadir)\" \
	-DGLADE_DIR=\"$(gladedir)\" \
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"




libexec_PROGRAMS = battstat-applet-2


battstat_applet_2_SOURCES = \
	battstat.h \
	pixmaps.h \
	properties.c \
	battstat_applet.c \
	acpi-linux.c \
	acpi-linux.h


battstat_applet_2_LDADD = \
		     $(GNOME_APPLETS_LIBS) \
		     $(LIBGLADE_LIBS) \
		     $(top_builddir)/screen-exec/libscreen-exec.la \
		     $(APMLIB)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = battstat.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_BattstatApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_BattstatApplet.xml

EXTRA_DIST = 					\
	$(glade_DATA)				\
	GNOME_BattstatApplet.server.in.in	\
	$(ui_DATA)				\
	$(schemas_in_files)		\
	battstat.png

@INTLTOOL_SERVER_RULE@

