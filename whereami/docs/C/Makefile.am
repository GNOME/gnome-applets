## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/whereami_applet/C
doc_DATA =	\
	index.html	\
	topic.dat	\
	whereami-applet.sgml \
	whereami.sgml

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = whereami-applet.sgml whereami.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: whereami-applet/index.html
	-cp whereami-applet/index.html .

whereami-applet/index.html: $(srcdir)/whereami.sgml $(srcdir)/whereami-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/whereami-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/whereami-applet
	-cp $(srcdir)/whereami-applet/*.html $(distdir)/whereami-applet
	-cp $(srcdir)/whereami-applet/*.css  $(distdir)/whereami-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in whereami-applet/*.html whereami-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

whereami-applet.ps: whereami-applet.sgml whereami.sgml
	-db2ps $<

whereami-applet.rtf: whereami-applet.sgml whereami.sgml
	-db2rtf $<
