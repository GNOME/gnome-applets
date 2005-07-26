SUBDIRS = docs pixmaps po src 

serverdir = $(libdir)/bonobo/servers
server_DATA = GNOME_NetspeedApplet.server
server_in_files = $(server_DATA:=.in)
server_in_in_files = $(server_in_files:=.in)

@INTLTOOL_SERVER_RULE@

spec = $(PACKAGE).spec

EXTRA_DIST = GNOME_NetspeedApplet.server.in.in \
             $(spec) \
             intltool-update.in \
             intltool-merge.in \
	     intltool-extract.in \
             xmldocs.make \
             omf.make

all-local: $(spec) $(server_DATA)

$(server_in_files): $(server_in_in_files) Makefile
	sed s,@LIBEXECDIR@,$(libexecdir),g <$< >$@

distuninstallcheck_listfiles = find . -type f -print | grep -v '^\./var/scrollkeeper'
