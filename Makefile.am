NULL =

always_built_SUBDIRS =  \
	window-buttons \
	$(NULL)

SUBDIRS = \
	data \
	gnome-applets \
	help \
	po			\
	$(always_built_SUBDIRS)	\
	$(NULL)

DIST_SUBDIRS = \
	data \
	gnome-applets \
	help \
	po		\
	window-buttons	\
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
