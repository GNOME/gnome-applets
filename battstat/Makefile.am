
glade_DATA = battstat_applet.glade

ACPIINC= @ACPIINC@

if NEED_LIBAPM
if HAVE_LIBAPM
APMLIB = -lapm
APMDIR =
APMINC = 
else
APMLIB = apmlib/libapm.a
APMDIR = apmlib
APMINC = -I$(srcdir)/apmlib/
endif
else
APMLIB =
APMDIR =
APMINC = 
endif

SUBDIRS = docs sounds $(APMDIR)

DIST_SUBDIRS = docs sounds apmlib

INCLUDES =					\
	$(GNOME_APPLETS_CFLAGS)			\
	$(LIBGLADE_CFLAGS)			\
	$(LIBNOTIFY_CFLAGS)			\
	$(APMINC)				\
	$(ACPIINC)				\
	-DG_LOG_DOMAIN=\"battstat_applet\"


libexec_PROGRAMS = battstat-applet-2


battstat_applet_2_SOURCES = \
	battstat.h \
	pixmaps.h \
	properties.c \
	battstat_applet.c \
	power-management.c \
	acpi-linux.c \
	acpi-linux.h \
	acpi-freebsd.c \
	acpi-freebsd.h


battstat_applet_2_LDADD = \
		     $(GNOME_APPLETS_LIBS) \
		     $(LIBGLADE_LIBS) \
		     $(LIBNOTIFY_LIBS) \
		     $(APMLIB)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = battstat.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
endif

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_BattstatApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_BattstatApplet.xml

EXTRA_DIST = 			\
	$(glade_DATA)		\
	$(ui_DATA)		\
	GNOME_BattstatApplet.server.in.in	\
	$(schemas_in_files)


@INTLTOOL_SERVER_RULE@

