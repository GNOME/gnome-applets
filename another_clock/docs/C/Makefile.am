## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/anotherclock_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = anotherclock-applet.sgml anotherclock.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: anotherclock-applet/index.html
	-cp anotherclock-applet/index.html .

anotherclock-applet/index.html: $(srcdir)/anotherclock.sgml $(srcdir)/anotherclock-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/anotherclock-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/anotherclock-applet
	-cp $(srcdir)/anotherclock-applet/*.html $(distdir)/anotherclock-applet
	-cp $(srcdir)/anotherclock-applet/*.css  $(distdir)/anotherclock-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in anotherclock-applet/*.html anotherclock-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

anotherclock-applet.ps: anotherclock-applet.sgml another-clock.sgml
	-db2ps $<

anotherclock-applet.rtf: anotherclock-applet.sgml another-clock.sgml
	-db2rtf $<
