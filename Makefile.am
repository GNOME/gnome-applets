SUBDIRS = help icons po src

serverdir = $(libdir)/bonobo/servers
server_DATA = GNOME_NetspeedApplet.server
server_in_files = $(server_DATA:=.in)
server_in_in_files = $(server_in_files:=.in)
@INTLTOOL_SERVER_RULE@

distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper

spec = $(PACKAGE).spec
EXTRA_DIST = GNOME_NetspeedApplet.server.in.in \
             $(spec) \
             gnome-doc-utils.make \
             intltool-update.in \
             intltool-merge.in \
             intltool-extract.in

all-local: $(spec) $(server_DATA)

$(server_in_files): $(server_in_in_files) Makefile
	sed s,@LIBEXECDIR@,$(libexecdir),g <$< >$@

CLEANFILES = $(server_DATA)
DISTCLEANFILES = \
                 gnome-doc-utils.make \
                 intltool-extract \
                 intltool-merge \
                 intltool-update \
                 $(server_in_files)

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper
