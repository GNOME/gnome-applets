appletdir = $(datadir)/applets

Utilitydir       = $(appletdir)/Utility
Monitorsdir      = $(appletdir)/Monitors
Multimediadir    = $(appletdir)/Multimedia
Networkdir       = $(appletdir)/Network
Amusementsdir    = $(appletdir)/Amusements

Directory_files = \
	Utility.directory \
	Monitor.directory \
	Multimedia.directory \
	Amusements.directory \
	Network.directory

## We explicitly define these so that the appropriate directories will
## be made at install time.
Utility_DATA =
Monitors_DATA =
Multimedia_DATA =
Network_DATA =
Amusements_DATA =

EXTRA_DIST = $(Utility_DATA) \
	     $(Monitors_DATA) \
	     $(Multimedia_DATA) \
	     $(Network_DATA) \
	     $(Amusements_DATA) \
	     $(Directory_files)

install-data-local:
	$(mkinstalldirs) $(Utilitydir)
	$(mkinstalldirs) $(Monitorsdir)
	$(mkinstalldirs) $(Multimediadir)
	$(mkinstalldirs) $(Networkdir)
	$(mkinstalldirs) $(Amusementsdir)
	$(INSTALL_DATA) $(srcdir)/Utility.directory $(Utilitydir)/.directory
	$(INSTALL_DATA) $(srcdir)/Monitors.directory $(Monitorsdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Multimedia.directory $(Multimediadir)/.directory
	$(INSTALL_DATA) $(srcdir)/Network.directory $(Networkdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Amusements.directory $(Amusementsdir)/.directory
