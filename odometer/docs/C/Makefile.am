## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/odometer_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = odometer-applet.sgml odometer.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: odometer-applet/index.html
	-cp odometer-applet/index.html .

odometer-applet/index.html: $(srcdir)/odometer.sgml $(srcdir)/odometer-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/odometer-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/odometer-applet
	-cp $(srcdir)/odometer-applet/*.html $(distdir)/odometer-applet
	-cp $(srcdir)/odometer-applet/*.css  $(distdir)/odometer-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in odometer-applet/*.html odometer-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

odometer-applet.ps: odometer-applet.sgml odometer.sgml
	-db2ps $<

odometer-applet.rtf: odometer-applet.sgml odometer.sgml
	-db2rtf $<
