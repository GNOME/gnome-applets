SUBDIRS = art

# ******************************************************************************
# Panel applet bonobo stuff
# ******************************************************************************
serverdir = $(libdir)/bonobo/servers
server_in_files = Invest_Applet.server.in
server_DATA = $(server_in_files:.server.in=.server)

# ******************************************************************************
# Desktop fdo launcher
# ******************************************************************************
desktopdir = $(datadir)/applications
desktop_in_files = invest-chart.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

# ******************************************************************************
# GConf Schemas
# ******************************************************************************
# GConf Preference Schemas
#schemadir = $(GCONF_SCHEMA_FILE_DIR)
#schema_in_files = deskbar-applet.schemas.in
#schema_DATA = $(schema_in_files:.schemas.in=.schemas)

# ******************************************************************************
# Misc data
# ******************************************************************************
resourcesdir = $(pkgdatadir)
resources_DATA = \
	Invest_Applet.xml \
	prefs-dialog.glade \
	financialchart.glade

investbindir = $(libdir)/invest-applet

# ******************************************************************************
# Build rules
# ******************************************************************************
@INTLTOOL_SERVER_RULE@
@INTLTOOL_SCHEMAS_RULE@
@INTLTOOL_DESKTOP_RULE@

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(investbindir)|" $< > $@

#if GCONF_SCHEMAS_INSTALL
#install-data-local: install-schemas
#install-schemas:
#	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
#		$(GCONFTOOL) --makefile-install-rule $(schema_DATA)
#endif

DISTCLEANFILES = \
	$(server_DATA) \
	$(server_in_files) \
	$(desktop_DATA)	
#	$(schema_DATA)

EXTRA_DIST = \
	Invest_Applet.server.in.in \
	$(resources_DATA) \
	$(desktop_in_files)
#	$(schema_in_files) 
	
	
