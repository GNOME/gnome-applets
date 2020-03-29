NULL =

if APPLET_ACCESSX
accessx_status_SUBDIR = accessx-status
endif

if HAVE_TRACKER_SEARCH_BAR
tracker_search_bar_SUBDIR = tracker-search-bar
endif

always_built_SUBDIRS =  \
	charpick	\
	drivemount	\
	inhibit		\
	geyes		\
	mini-commander \
	multiload \
	sticky-notes	\
	window-buttons \
	window-title \
	$(NULL)

SUBDIRS = \
	data \
	gnome-applets \
	help \
	po			\
	$(always_built_SUBDIRS)	\
	$(vfs_SUBDIRS)		\
	$(accessx_status_SUBDIR)\
	$(tracker_search_bar_SUBDIR) \
	$(NULL)

DIST_SUBDIRS = \
	data \
	gnome-applets \
	help \
	po		\
	drivemount	\
	charpick	\
	geyes		\
	mini-commander	\
	multiload	\
	accessx-status	\
	sticky-notes	\
	window-buttons	\
	window-title	\
	inhibit		\
	tracker-search-bar \
	$(NULL)

EXTRA_DIST = \
	COPYING-DOCS \
	$(NULL)

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-libpanel-applet-dir='$${datadir}/gnome-panel/distcheck/applets' \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_GETTEXT) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	`find "$(srcdir)/m4" -type f -name "*.m4" -print` \
	$(srcdir)/INSTALL \
	$(srcdir)/config.h.in~ \
	$(srcdir)/configure \
	$(NULL)

GITIGNOREFILES = $(PACKAGE)-\*.tar.{gz,bz2,xz}

-include $(top_srcdir)/git.mk
