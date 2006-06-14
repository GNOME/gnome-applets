SUBDIRS = help pixmaps src

schemasdir       = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = cpufreq-applet.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

serverdir          = $(libdir)/bonobo/servers
server_in_files    = GNOME_CPUFreqApplet.server.in
server_in_in_files = $(server_in_files:.server.in=.server.in.in)
server_DATA        = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_in_files)
	sed -e "s|\@DATADIR\@|$(datadir)|;s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

glade_DATA = cpufreq-preferences.glade

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_CPUFreqApplet.xml

if GCONF_SCHEMAS_INSTALL
install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/cpufreq/$(schemas_DATA) ;
endif

CLEANFILES = \
	     GNOME_CPUFreqApplet.server         \
	     GNOME_CPUFreqApplet.server.in      \
	     cpufreq-applet.schemas             \
	     *.bak                              \
	     *.gladep

EXTRA_DIST = \
	     $(server_in_in_files)           \
	     $(schemas_in_files)             \
	     $(glade_DATA)                   \
	     $(ui_DATA)
	     
					

