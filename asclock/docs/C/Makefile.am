## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/asclock_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = asclock-applet.sgml asclock.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: asclock-applet/index.html
	-cp asclock-applet/index.html .

asclock-applet/index.html: $(srcdir)/asclock.sgml $(srcdir)/asclock-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/asclock-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/asclock-applet
	-cp $(srcdir)/asclock-applet/*.html $(distdir)/asclock-applet
	-cp $(srcdir)/asclock-applet/*.css  $(distdir)/asclock-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in asclock-applet/*.html asclock-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

asclock-applet.ps: asclock-applet.sgml asclock.sgml
	-db2ps $<

asclock-applet.rtf: asclock-applet.sgml asclock.sgml
	-db2rtf $<
