## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/fifteen_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = fifteen-applet.sgml fifteen.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: fifteen-applet/index.html
	-cp fifteen-applet/index.html .

fifteen-applet/index.html: $(srcdir)/fifteen.sgml $(srcdir)/fifteen-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/fifteen-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/fifteen-applet
	-cp $(srcdir)/fifteen-applet/*.html $(distdir)/fifteen-applet
	-cp $(srcdir)/fifteen-applet/*.css  $(distdir)/fifteen-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in fifteen-applet/*.html fifteen-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

fifteen-applet.ps: fifteen-applet.sgml fifteen.sgml
	-db2ps $<

fifteen-applet.rtf: fifteen-applet.sgml fifteen.sgml
	-db2rtf $<
