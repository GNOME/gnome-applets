## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/geyes_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = geyes-applet.sgml geyes.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: geyes-applet/index.html
	-cp geyes-applet/index.html .

geyes-applet/index.html: $(srcdir)/geyes.sgml $(srcdir)/geyes-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/geyes-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/geyes-applet
	-cp $(srcdir)/geyes-applet/*.html $(distdir)/geyes-applet
	-cp $(srcdir)/geyes-applet/*.css  $(distdir)/geyes-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in geyes-applet/*.html geyes-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

geyes-applet.ps: geyes-applet.sgml geyes.sgml
	-db2ps $<

geyes-applet.rtf: geyes-applet.sgml geyes.sgml
	-db2rtf $<
