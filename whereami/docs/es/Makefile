# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


# To use this template:
#     1) Define: figs, docname, lang, omffile, sgml_ents although figs, 
#        omffile, and sgml_ents may be empty in your Makefile.am which 
#        will "include" this one 
#     2) Figures must go under figures/ and be in PNG format
#     3) You should only have one document per directory 
#
#        Note that this makefile forces the directory name under
#        $prefix/share/gnome/help/ to be the same as the SGML filename
#        of the document.  This is required by GNOME. eg:
#        $prefix/share/gnome/help/fish_applet/C/fish_applet.sgml
#                                 ^^^^^^^^^^^   ^^^^^^^^^^^
# Definitions:
#   figs         A list of screenshots which will be included in EXTRA_DIST
#                Note that these should reside in figures/ and should be .png
#                files, or you will have to make modifications below.
#   docname      This is the name of the SGML file: <docname>.sgml
#   lang         This is the document locale
#   omffile      This is the name of the OMF file.  Convention is to name
#                it <docname>-<locale>.omf.
#   sgml_ents    This is a list of SGML entities which must be installed 
#                with the main SGML file and included in EXTRA_DIST. 
# eg:
#   figs = \
#          figures/fig1.png            \
#          figures/fig2.png
#   docname = scrollkeeper-manual
#   lang = C
#   omffile=scrollkeeper-manual-C.omf
#   sgml_ents = fdl.sgml
#   include $(top_srcdir)/help/sgmldocs.make
#   dist-hook: app-dist-hook
#


SHELL = /bin/sh

srcdir = .
top_srcdir = ../../..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/gnome-applets
pkglibdir = $(libdir)/gnome-applets
pkgincludedir = $(includedir)/gnome-applets

top_builddir = ../../..

ACLOCAL = aclocal -I macros
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
AS = @AS@
BUILD_INCLUDED_LIBINTL = no
CANVAS_PIXBUF_CFLAGS = -I/usr/include -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include
CANVAS_PIXBUF_LIBS = -rdynamic -L/usr/lib -L/usr/X11R6/lib -lgdk_pixbuf -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm -lgnomecanvaspixbuf
CATALOGS =  az.gmo ca.gmo cs.gmo da.gmo de.gmo el.gmo en_GB.gmo es.gmo et.gmo eu.gmo fi.gmo fr.gmo ga.gmo gl.gmo hu.gmo it.gmo ja.gmo ko.gmo lt.gmo nl.gmo nn.gmo no.gmo pl.gmo pt.gmo pt_BR.gmo ro.gmo ru.gmo sk.gmo sl.gmo sv.gmo tr.gmo uk.gmo wa.gmo zh_CN.GB2312.gmo
CATOBJEXT = .gmo
CC = gcc
CDROM_HOST = linux
CDROM_LIBS = 
CFLAGS = -g -O2 -Wall -Wunused -Wmissing-prototypes -Wmissing-declarations 
CPPFLAGS =  -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include
DATADIRNAME = share
DLLTOOL = @DLLTOOL@
ESD_CFLAGS = 
ESD_CONFIG = /usr/bin/esd-config
ESD_LIBS = -L/usr/lib -lesd -laudiofile -lm
GENCAT = 
GHTTP_LIB = -lghttp 
GMOFILES =  az.gmo ca.gmo cs.gmo da.gmo de.gmo el.gmo en_GB.gmo es.gmo et.gmo eu.gmo fi.gmo fr.gmo ga.gmo gl.gmo hu.gmo it.gmo ja.gmo ko.gmo lt.gmo nl.gmo nn.gmo no.gmo pl.gmo pt.gmo pt_BR.gmo ro.gmo ru.gmo sk.gmo sl.gmo sv.gmo tr.gmo uk.gmo wa.gmo zh_CN.GB2312.gmo
GMSGFMT = /usr/bin/msgfmt
GNOMEGNORBA_LIBS = -rdynamic -lgnorba -lORBitCosNaming -lORBit -lIIOP -lORBitutil -lnsl -lgnomeui -lart_lgpl -lgdk_imlib -lSM -lICE -lgtk -lgdk -lgmodule -lXi -lXext -lX11 -lgnome -lgnomesupport -lesd -laudiofile -lm -ldb1 -lglib -ldl
GNOMEUI_LIBS = -rdynamic -lgnomeui -lart_lgpl -lgdk_imlib -lSM -lICE -lgtk -lgdk -lgmodule -lXi -lXext -lX11 -lgnome -lgnomesupport -lesd -laudiofile -lm -ldb1 -lglib -ldl
GNOME_APPLETS_LIBS = -rdynamic -lpanel_applet -lgnorba -lORBitCosNaming -lORBit -lIIOP -lORBitutil -lnsl -lgnomeui -lart_lgpl -lgdk_imlib -lSM -lICE -lgtk -lgdk -lgmodule -lXi -lXext -lX11 -lgnome -lgnomesupport -lesd -laudiofile -lm -ldb1 -lglib -ldl
GNOME_CAPPLET_LIBS = 
GNOME_CONFIG = /usr/bin/gnome-config
GNOME_DOCKLETS_LIBS = 
GNOME_INCLUDEDIR = -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/include/gtk-1.2 -I/usr/X11R6/include -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/include/gtk-1.2 -I/usr/X11R6/include
GNOME_LIBDIR = -rdynamic -L/usr/lib -L/usr/X11R6/lib
GNOME_LIBS = -lgnome -lgnomesupport -lesd -laudiofile -lm -ldb1 -lglib -ldl
GNOME_XML_CFLAGS = -I/usr/include/gnome-xml
GNOME_XML_LIB = -L/usr/lib -lxml -lz
GNORBA_CFLAGS = -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/include/gtk-1.2 -I/usr/X11R6/include
GNORBA_LIBS = -rdynamic -L/usr/lib -L/usr/X11R6/lib -lgnorba -lORBitCosNaming -lORBit -lIIOP -lORBitutil -lnsl -lgnomeui -lart_lgpl -lgdk_imlib -lSM -lICE -lgtk -lgdk -lgmodule -lXi -lXext -lX11 -lgnome -lgnomesupport -lesd -laudiofile -lm -ldb1 -lglib -ldl
GTKXMHTML_LIBS = -rdynamic -lgtkxmhtml -lXpm -ljpeg -lpng -lz -lSM -lICE -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
GTK_CFLAGS = -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include
GTK_CONFIG = /usr/bin/gtk-config
GTK_LIBS = -lSM -lICE -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
GT_NO = 
GT_YES = #YES#
HAVE_JW = yes
INCLUDE_LOCALE_H = #include <locale.h>
INSTOBJEXT = .mo
INTLDEPS = 
INTLLIBS = 
INTLOBJS = 
JW = /usr/bin/jw
LDFLAGS = 
LIBGTOP_BINARY_AGE = 
LIBGTOP_BINDIR = /usr/bin
LIBGTOP_CONFIG = /usr/bin/libgtop-config
LIBGTOP_EXTRA_LIBS = -L/usr/X11R6/lib -lXau -L/usr/lib -lglib
LIBGTOP_INCLUDEDIR = /usr/include
LIBGTOP_INCS = -I/usr/include -DHAVE_LIBGTOP_SMP -DHAVE_LIBGTOP_SYSINFO -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/include/gtk-1.2 -I/usr/X11R6/include -I/usr/include/glib-1.2 -I/usr/lib/glib/include
LIBGTOP_INTERFACE_AGE = 
LIBGTOP_LIBDIR = /usr/lib
LIBGTOP_LIBS = -L/usr/lib -lgtop -lgtop_sysdeps -lgtop_common
LIBGTOP_MAJOR_VERSION = 1
LIBGTOP_MICRO_VERSION = 12
LIBGTOP_MINOR_VERSION = 0
LIBGTOP_NAMES_INCS = -I/usr/include -DHAVE_LIBGTOP_SMP -DHAVE_LIBGTOP_SYSINFO -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/include/gtk-1.2 -I/usr/X11R6/include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -DGLIBTOP_NAMES
LIBGTOP_NAMES_LIBS = -L/usr/lib -lgtop -lgtop_sysdeps -lgtop_common -lgtop_names
LIBGTOP_SERVER = /usr/bin/libgtop_server
LIBGTOP_SERVER_VERSION = 5
LIBGTOP_VERSION = 1.0.12
LIBGTOP_VERSION_CODE = 1000012
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LN_S = ln -s
MAINT = 
MAKEINFO = makeinfo
MIXER_LIBS = 
MKINSTALLDIRS = ./mkinstalldirs
OBJDUMP = @OBJDUMP@
ORBIT_CFLAGS = -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/include
ORBIT_CONFIG = /usr/bin/orbit-config
ORBIT_IDL = /usr/bin/orbit-idl
ORBIT_LIBS = -L/usr/lib -lORBitCosNaming -lORBit -lIIOP -lORBitutil -lglib -lnsl -lm
PACKAGE = gnome-applets
PIXBUF_CFLAGS = -I/usr/include -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include
PIXBUF_LIBS = -rdynamic -L/usr/lib -L/usr/X11R6/lib -lgdk_pixbuf -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
POFILES =  az.po ca.po cs.po da.po de.po el.po en_GB.po es.po et.po eu.po fi.po fr.po ga.po gl.po hu.po it.po ja.po ko.po lt.po nl.po nn.po no.po pl.po pt.po pt_BR.po ro.po ru.po sk.po sl.po sv.po tr.po uk.po wa.po zh_CN.GB2312.po
POSUB = po
PTHREAD_LIB = -lpthread
RANLIB = ranlib
SCROLLKEEPER_CONFIG = /usr/bin/scrollkeeper-config
SCROLLKEEPER_REQUIRED = 0.1.4
USE_INCLUDED_LIBINTL = no
USE_NLS = yes
VERSION = 1.4.0.2
XML_I18N_EXTRACT = $(top_builddir)/xml-i18n-extract
XML_I18N_MERGE = $(top_builddir)/xml-i18n-merge
XML_I18N_TOOLS_PERL = /usr/local/bin/perl
XML_I18N_UPDATE = $(top_builddir)/xml-i18n-update
XPM_LIBS = -lXpm
ZVT_LIBS = -rdynamic -lzvt -lutil -lSM -lICE -lgdk_imlib -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
cflags_set = yes
cxxflags_set = @cxxflags_set@
gnomelocaledir = ${prefix}/${DATADIRNAME}/locale
l = 

figs =  	figures/whereami_applet.png

docname = whereami_applet
lang = es
omffile = whereami_applet-es.omf
sgml_ents = whereami.sgml 	license.sgml 	licencia-es.sgml 	licencia-nota-es.sgml 	authors.sgml 


docdir = $(datadir)/gnome/help/$(docname)/$(lang)

doc_DATA = index.html

sgml_files = $(sgml_ents) $(docname).sgml

omf_dir = $(top_srcdir)/omf-install

EXTRA_DIST = $(sgml_files) $(doc_DATA) $(omffile) $(figs)

CLEANFILES = omf_timestamp
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../../config.h
CONFIG_CLEAN_FILES = 
DATA =  $(doc_DATA)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in:  Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) $(top_srcdir)/sgmldocs.make
	cd $(top_srcdir) && $(AUTOMAKE) --gnu whereami/docs/es/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


install-docDATA: $(doc_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	@list='$(doc_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(docdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(docdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(docdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(docdir)/$$p; \
	  fi; fi; \
	done

uninstall-docDATA:
	@$(NORMAL_UNINSTALL)
	list='$(doc_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(docdir)/$$p; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = whereami/docs/es

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu whereami/docs/es/Makefile
	$(mkinstalldirs) $(distdir)/figures
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
	$(MAKE) $(AM_MAKEFLAGS) top_distdir="$(top_distdir)" distdir="$(distdir)" dist-hook
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am: install-docDATA
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-docDATA uninstall-local
uninstall: uninstall-am
all-am: Makefile $(DATA)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(docdir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am
	-rm -f libtool

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: uninstall-docDATA install-docDATA tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-local uninstall-am uninstall all-redirect all-am all \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


all: index.html omf

omf: omf_timestamp

omf_timestamp: $(omffile)
	-for file in $(omffile); do \
	  scrollkeeper-preinstall $(docdir)/$(docname).sgml $$file $(omf_dir)/$$file; \
	done
	touch omf_timestamp

index.html: $(docname)/index.html
	-cp $(docname)/index.html .

$(docname).sgml: $(sgml_ents)
        -ourdir=`pwd`;  \
        cd $(srcdir);   \
        cp $(sgml_ents) $$ourdir

# The weird srcdir trick is because the db2html from the Cygnus RPMs
# cannot handle relative filenames
$(docname)/index.html: $(srcdir)/$(docname).sgml
	-srcdir=`cd $(srcdir) && pwd`;			\
	if test "$(HAVE_JW)" = 'yes' ; then 		\
		jw -c /etc/sgml/catalog $$srcdir/$(docname).sgml -o $$srcdir/$(docname); \
	else 						\
		db2html $$srcdir/$(docname).sgml; 	\
	 fi

app-dist-hook: index.html
	-$(mkinstalldirs) $(distdir)/$(docname)/stylesheet-images
	-$(mkinstalldirs) $(distdir)/figures
	-cp $(srcdir)/$(docname)/*.html $(distdir)/$(docname)
	-for file in $(srcdir)/$(docname)/*.css; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  cp $$file $(distdir)/$(docname)/$$basefile ; \
	done
	-for file in $(srcdir)/$(docname)/stylesheet-images/*.gif; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  cp $$file $(distdir)/$(docname)/stylesheet-images/$$basefile ; \
	done
	-if [ -e topic.dat ]; then \
		cp $(srcdir)/topic.dat $(distdir); \
	 fi

install-data-am: index.html omf
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/stylesheet-images
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/figures
	-cp $(srcdir)/$(sgml_files) $(DESTDIR)$(docdir)
	-for file in $(srcdir)/$(docname)/*.html $(srcdir)/$(docname)/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/figures/*.png; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/figures/$$basefile; \
	done
	-for file in $(srcdir)/$(docname)/stylesheet-images/*.gif; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/stylesheet-images/$$basefile; \
	done
	-if [ -e $(srcdir)/topic.dat ]; then \
		$(INSTALL_DATA) $(srcdir)/topic.dat $(DESTDIR)$(docdir); \
	 fi

$(docname).ps: $(srcdir)/$(docname).sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2ps $$srcdir/$(docname).sgml

$(docname).rtf: $(srcdir)/$(docname).sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2ps $$srcdir/$(docname).sgml

uninstall-local:
	-for file in $(srcdir)/$(docname)/stylesheet-images/*.gif; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  rm -f $(docdir)/stylesheet-images/$$basefile; \
	done
	-for file in $(srcdir)/figures/*.png; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  rm -f $(docdir)/figures/$$basefile; \
	done
	-for file in $(srcdir)/$(docname)/*.html $(srcdir)/$(docname)/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  rm -f $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(sgml_files); do \
	  rm -f $(DESTDIR)$(docdir)/$$file; \
	done
	-rmdir $(DESTDIR)$(docdir)/stylesheet-images
	-rmdir $(DESTDIR)$(docdir)/figures
	-rmdir $(DESTDIR)$(docdir)
dist-hook: app-dist-hook

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
