## Process this file with automake to produce Makefile.in

figs =

docdir = $(datadir)/gnome/help/cpumemusage_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = cpumemusage-applet.sgml cpumemusage.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: cpumemusage-applet/index.html
	-cp cpumemusage-applet/index.html .

cpumemusage-applet/index.html: $(srcdir)/cpumemusage.sgml $(srcdir)/cpumemusage-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/cpumemusage-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/cpumemusage-applet
	-cp $(srcdir)/cpumemusage-applet/*.html $(distdir)/cpumemusage-applet
	-cp $(srcdir)/cpumemusage-applet/*.css  $(distdir)/cpumemusage-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in cpumemusage-applet/*.html cpumemusage-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

cpumemusage-applet.ps: cpumemusage-applet.sgml cpumemusage.sgml
	-db2ps $<

cpumemusage-applet.rtf: cpumemusage-applet.sgml cpumemusage.sgml
	-db2rtf $<
