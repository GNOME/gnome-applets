appletdir = $(datadir)/applets

Utilitydir       = $(appletdir)/Utility
Monitorsdir      = $(appletdir)/Monitors
Multimediadir    = $(appletdir)/Multimedia
Networkdir       = $(appletdir)/Network
Amusementsdir    = $(appletdir)/Amusements

Directory_files = \
	Utility.directory \
	Monitors.directory \
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
	$(mkinstalldirs) $(DESTDIR)$(Utilitydir)
	$(mkinstalldirs) $(DESTDIR)$(Monitorsdir)
	$(mkinstalldirs) $(DESTDIR)$(Multimediadir)
	$(mkinstalldirs) $(DESTDIR)$(Networkdir)
	$(mkinstalldirs) $(DESTDIR)$(Amusementsdir)
	$(INSTALL_DATA) $(srcdir)/Utility.directory $(DESTDIR)$(Utilitydir)/.directory
	$(INSTALL_DATA) $(srcdir)/Monitors.directory $(DESTDIR)$(Monitorsdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Multimedia.directory $(DESTDIR)$(Multimediadir)/.directory
	$(INSTALL_DATA) $(srcdir)/Network.directory $(DESTDIR)$(Networkdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Amusements.directory $(DESTDIR)$(Amusementsdir)/.directory
