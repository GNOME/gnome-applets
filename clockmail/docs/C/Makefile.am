## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/clockmail_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = clockmail-applet.sgml clockmail.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: clockmail-applet/index.html
	-cp clockmail-applet/index.html .

clockmail-applet/index.html: $(srcdir)/clockmail.sgml $(srcdir)/clockmail-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/clockmail-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/clockmail-applet
	-cp $(srcdir)/clockmail-applet/*.html $(distdir)/clockmail-applet
	-cp $(srcdir)/clockmail-applet/*.css  $(distdir)/clockmail-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in clockmail-applet/*.html clockmail-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

clockmail-applet.ps: clockmail-applet.sgml clockmail.sgml
	-db2ps $<

clockmail-applet.rtf: clockmail-applet.sgml clockmail.sgml
	-db2rtf $<
