## Process this file with automake to produce Makefile.in

# Taken from gnome-applets/mini-commander/docs/Makefile.am

docdir = $(datadir)/gnome/help/gweather/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = gweather.sgml topic.dat

# This is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: gweather/index.html
	-cp gweather/index.html .

gweather/index.html: $(srcdir)/gweather.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/gweather.sgml

dist-hook: index.html
	mkdir $(distdir)/gweather
	-cp $(srcdir)/gweather/*.html $(distdir)/gweather
	-cp $(srcdir)/gweather/*.css $(distdir)/gweather

install-data-local: index.html
	-for file in gweather/*.html gweather/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done

gweather.ps: gweather.sgml
	-db2ps $<

gweather.rtf: gweather.sgml
	-db2rtf $<
