## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/multiload_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = multiload-applet.sgml cpuload.sgml memload.sgml loadavg.sgml netload.sgml swapload.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: multiload-applet/index.html
	-cp multiload-applet/index.html .

multiload-applet/index.html: $(srcdir)/cpuload.sgml $(srcdir)/memload.sgml $(srcdir)/loadavg.sgml $(srcdir)/netload.sgml $(srcdir)/swapload.sgml $(srcdir)/multiload-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/multiload-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/multiload-applet
	-cp $(srcdir)/multiload-applet/*.html $(distdir)/multiload-applet
	-cp $(srcdir)/multiload-applet/*.css  $(distdir)/multiload-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in multiload-applet/*.html multiload-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

multiload-applet.ps: multiload-applet.sgml multiload.sgml
	-db2ps $<

multiload-applet.rtf: multiload-applet.sgml multiload.sgml
	-db2rtf $<
