## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/jbc_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = jbc-applet.sgml jbc.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: jbc-applet/index.html
	-cp jbc-applet/index.html .

jbc-applet/index.html: $(srcdir)/jbc.sgml $(srcdir)/jbc-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/jbc-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/jbc-applet
	-cp $(srcdir)/jbc-applet/*.html $(distdir)/jbc-applet
	-cp $(srcdir)/jbc-applet/*.css  $(distdir)/jbc-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in jbc-applet/*.html jbc-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

jbc-applet.ps: jbc-applet.sgml jbc.sgml
	-db2ps $<

jbc-applet.rtf: jbc-applet.sgml jbc.sgml
	-db2rtf $<
