## Process this file with automake to produce Makefile.in

docdir = $(datadir)/gnome/help/modemlights_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = modemlights.sgml topic.dat

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: modemlights/modemlights.html
	-cp modemlights/modemlights.html index.html

modemlights/index.html: $(srcdir)/modemlights.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/modemlights.sgml

dist-hook: index.html
	mkdir $(distdir)/modemlights
	-cp $(srcdir)/modemlights/*.html $(distdir)/modemlights
	-cp $(srcdir)/modemlights/*.css $(distcidr)/modemlights

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in modemlights/*.html modemlights/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/fig/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

gnome-terminal.ps: gnome-terminal.sgml
	-db2ps $<

gnome-terminal.rtf: gnome-terminal.sgml
	-db2rtf $<
