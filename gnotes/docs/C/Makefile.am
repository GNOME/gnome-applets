## Process this file with automake to produce Makefile.in

figs = \
	gnotes-fig.png	\
	gnotes-properties.png


docdir = $(datadir)/gnome/help/gnotes_applet/C
doc_DATA =	\
	index.html	\
	topic.dat	\
	$(figs)

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = gnotes-applet.sgml gnotes.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: gnotes-applet/index.html
	-cp gnotes-applet/index.html .

gnotes-applet/index.html: $(srcdir)/gnotes.sgml $(srcdir)/gnotes-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/gnotes-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/gnotes-applet
	-cp $(srcdir)/gnotes-applet/*.html $(distdir)/gnotes-applet
	-cp $(srcdir)/gnotes-applet/*.css  $(distdir)/gnotes-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in gnotes-applet/*.html gnotes-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

gnotes-applet.ps: gnotes-applet.sgml gnotes.sgml
	-db2ps $<

gnotes-applet.rtf: gnotes-applet.sgml gnotes.sgml
	-db2rtf $<
