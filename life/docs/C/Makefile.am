## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/life_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = life-applet.sgml life.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: life-applet/index.html
	-cp life-applet/index.html .

life-applet/index.html: $(srcdir)/life.sgml $(srcdir)/life-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/life-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/life-applet
	-cp $(srcdir)/life-applet/*.html $(distdir)/life-applet
	-cp $(srcdir)/life-applet/*.css  $(distdir)/life-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in life-applet/*.html life-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

life-applet.ps: life-applet.sgml life.sgml
	-db2ps $<

life-applet.rtf: life-applet.sgml life.sgml
	-db2rtf $<
