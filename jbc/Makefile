# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.3 from Makefile.am

# Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# trick Automake into defining $(CXXLINK)


SHELL = /bin/sh

srcdir = .
top_srcdir = ../..
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

DISTDIR =

pkgdatadir = $(datadir)/gnome-core
pkglibdir = $(libdir)/gnome-core
pkgincludedir = $(includedir)/gnome-core

top_builddir = ../..

ACLOCAL = aclocal -I macros
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
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
CATALOGS =  it.gmo ko.gmo fr.gmo de.gmo es.gmo no.gmo ga.gmo sv.gmo pt.gmo ja.gmo fi.gmo
CATOBJEXT = .gmo
CC = gcc
CDROM_HOST = linux
CFLAGS = -g -O2
CPPFLAGS =  -I/usr/X11R6/include -I/usr/local/lib/glib/include -I/usr/local/include
CXX = c++
DATADIRNAME = share
DL_LIB = -ldl
GENCAT = 
GMOFILES =  it.gmo ko.gmo fr.gmo de.gmo es.gmo no.gmo ga.gmo sv.gmo pt.gmo ja.gmo fi.gmo
GMSGFMT = /usr/bin/msgfmt
GNOMEUI_LIBS = -rdynamic -lgnomeui -lgdk_imlib -ltiff -lgif -ljpeg -lpng -lz -lSM -lICE -lgtk-1.1 -lgdk-1.1 -lgmodule-1.1 -lXext -lX11 -lm -lgnome -lgnomesupport -lglib-1.1 -ldl
GNOME_CONFIG = /usr/local/bin/gnome-config
GNOME_INCLUDEDIR = -I/usr/local/include -DNEED_GNOMESUPPORT_H -I/usr/local/lib/gnome-libs/include -I/usr/X11R6/include -I/usr/local/lib/glib/include
GNOME_LIBDIR = -rdynamic -L/usr/local/lib -L/usr/X11R6/lib
GNOME_LIBS = -lgnome -lgnomesupport -lglib-1.1 -ldl
GTKXMHTML_LIBS = -rdynamic -lgtkxmhtml -lXpm -ljpeg -lpng -lz -lSM -lICE -lgtk-1.1 -lgdk-1.1 -lgmodule-1.1 -lglib-1.1 -ldl -lXext -lX11 -lm
GTK_CFLAGS = -I/usr/X11R6/include -I/usr/local/lib/glib/include -I/usr/local/include
GTK_CONFIG = /usr/local/bin/gtk-config
GTK_LIBS = -lSM -lICE -L/usr/local/lib -L/usr/X11R6/lib -lgtk-1.1 -lgdk-1.1 -rdynamic -lgmodule-1.1 -lglib-1.1 -ldl -lXext -lX11 -lm
GT_NO = 
GT_YES = #YES#
INCLUDE_LOCALE_H = #include <locale.h>
INSTOBJEXT = .mo
INTLDEPS = 
INTLLIBS = 
INTLOBJS = 
LD = /usr/bin/ld
LDFLAGS = 
LEX = flex
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LN_S = ln -s
MAINT = 
MAKEINFO = makeinfo
MKINSTALLDIRS = ./mkinstalldirs
MSGFMT = /usr/bin/msgfmt
NM = /usr/bin/nm -B
ORBIT_CFLAGS = -I/usr/local/lib/glib/include -I/usr/local/include
ORBIT_CONFIG = orbit-config
ORBIT_LIBS = -L/usr/local/lib -lORBit -lIIOP -lORBitutil -lglib-1.1 -lm
ORB_CFLAGS = -I/usr/local/lib/glib/include -I/usr/local/include
ORB_LIBS = -L/usr/local/lib -lORBit -lIIOP -lORBitutil -lglib-1.1 -lm
PACKAGE = gnome-core
POFILES =  it.po ko.po fr.po de.po es.po no.po ga.po sv.po pt.po ja.po fi.po
POSUB = po
PTHREAD_LIB = -lpthread
RANLIB = ranlib
RES_LIBS = 
SMPROXY_DEFS = -Dlinux -D__i386__ -D_POSIX_C_SOURCE=199309L -D_POSIX_SOURCE -D_XOPEN_SOURCE=500L -D_BSD_SOURCE -D_SVID_SOURCE 
USE_INCLUDED_LIBINTL = no
USE_NLS = yes
VERSION = 0.25
WINDOW_MANAGER = icewm
XF86MISC_LIBS = -lXxf86misc
XPM_LIBS = -lXpm
YACC = bison -y
l = 

#BUILT_SOURCES = dummy.cc

INCLUDES = -I. -I$(srcdir) 				\
	-I$(srcdir)/../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = jbc_applet

jbc_applet_SOURCES = jbc.c

jbc_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) 

jbc_applet_LINK = $(CXXLINK)

EXTRA_DIST = \
	jbc_applet.desktop	\
sysdir = $(datadir)/applets/Amusements

sys_DATA = jbc_applet.desktop
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../config.h
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I../..
LIBS = 
jbc_applet_OBJECTS =  jbc.o
jbc_applet_DEPENDENCIES =  ../../panel/libpanel_applet.la
jbc_applet_LDFLAGS = 
CXXFLAGS = -g -O2
CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS)
LTCXXCOMPILE = $(LIBTOOL) --mode=compile $(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS)
CXXLINK = $(LIBTOOL) --mode=link $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
LTCOMPILE = $(LIBTOOL) --mode=compile $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
LINK = $(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(LDFLAGS) -o $@
DIST_COMMON =  README ChangeLog Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP = --best
DEP_FILES =  .deps/dummy .deps/jbc.P
CXXMKDEP = $(CXX) -M $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS)
SOURCES = $(jbc_applet_SOURCES)
OBJECTS = $(jbc_applet_OBJECTS)

all: Makefile $(PROGRAMS)

.SUFFIXES:
.SUFFIXES: .S .c .cc .lo .o .s
$(srcdir)/Makefile.in:  Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4)
	cd $(top_srcdir) && $(AUTOMAKE) --gnu applets/jbc/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`"; \
	    $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	done

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

.s.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.S.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

mostlyclean-libtool:
	-rm -f *.lo

clean-libtool:
	-rm -rf .libs _libs

distclean-libtool:

maintainer-clean-libtool:

jbc_applet: $(jbc_applet_OBJECTS) $(jbc_applet_DEPENDENCIES)
	@rm -f jbc_applet
	$(jbc_applet_LINK) $(jbc_applet_LDFLAGS) $(jbc_applet_OBJECTS) $(jbc_applet_LDADD) $(LIBS)
.cc.o:
	$(CXXCOMPILE) -c $<
.cc.lo:
	$(LTCXXCOMPILE) -c $<

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $(SOURCES) $(HEADERS) $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = applets/jbc

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu applets/jbc/Makefile
	$(mkinstalldirs) $(distdir)/$(datadir)/applets
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  test -f $(distdir)/$$file \
	  || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	  || cp -p $$d/$$file $(distdir)/$$file; \
	done

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

-include $(DEP_FILES)

mostlyclean-depend:

clean-depend:

distclean-depend:

maintainer-clean-depend:
	-rm -rf .deps

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MD,.deps/$(*F).P -c $<

%.lo: %.c
	@echo '$(LTCOMPILE) -c $<'; \
	$(LTCOMPILE) -Wp,-MD,.deps/$(*F).p -c $<
	@-sed -e 's/^\([^:]*\)\.o:/\1.lo \1.o:/' \
	  < .deps/$(*F).p > .deps/$(*F).P
	@-rm -f .deps/$(*F).p

%.o: %.cc
	@echo '$(CXXCOMPILE) -c $<'; \
	$(CXXCOMPILE) -Wp,-MD,.deps/$(*F).P -c $<

%.lo: %.cc
	@echo '$(LTCXXCOMPILE) -c $<'; \
	$(LTCXXCOMPILE) -Wp,-MD,.deps/$(*F).p -c $<
	@-sed -e 's/^\([^:]*\)\.o:/\1.lo \1.o:/' \
	  < .deps/$(*F).p > .deps/$(*F).P
	@-rm -f .deps/$(*F).p
info:
dvi:
check: all
	$(MAKE)
installcheck:
install-exec: install-binPROGRAMS
	@$(NORMAL_INSTALL)

install-data: 
	@$(NORMAL_INSTALL)

install: install-exec install-data all
	@:

uninstall: uninstall-binPROGRAMS

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' INSTALL_SCRIPT='$(INSTALL_PROGRAM)' install
installdirs:
	$(mkinstalldirs)  $(DATADIR)$(bindir)


mostlyclean-generic:
	-test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(DISTCLEANFILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	-test -z "$(MAINTAINERCLEANFILES)" || rm -f $(MAINTAINERCLEANFILES)
	-test -z "$(BUILT_SOURCES)" || rm -f $(BUILT_SOURCES)
mostlyclean:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-libtool mostlyclean-tags mostlyclean-depend \
		mostlyclean-generic

clean:  clean-binPROGRAMS clean-compile clean-libtool clean-tags \
		clean-depend clean-generic mostlyclean

distclean:  distclean-binPROGRAMS distclean-compile distclean-libtool \
		distclean-tags distclean-depend distclean-generic clean
	-rm -f config.status
	-rm -f libtool

maintainer-clean:  maintainer-clean-binPROGRAMS maintainer-clean-compile \
		maintainer-clean-libtool maintainer-clean-tags \
		maintainer-clean-depend maintainer-clean-generic \
		distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile mostlyclean-libtool distclean-libtool \
clean-libtool maintainer-clean-libtool tags mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir \
mostlyclean-depend distclean-depend clean-depend \
maintainer-clean-depend info dvi installcheck install-exec install-data \
install uninstall all installdirs mostlyclean-generic distclean-generic \
clean-generic maintainer-clean-generic clean mostlyclean distclean \
maintainer-clean


#dummy.cc:
#	@touch dummy.cc

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
