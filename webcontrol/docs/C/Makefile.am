## Process this file with automake to produce Makefile.in

docdir = $(datadir)/gnome/help/webcontrol_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = webcontrol-applet.sgml topic.dat

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: webcontrol-applet/webcontrol-applet.html
	-cp webcontrol-applet/webcontrol-applet.html index.html

webcontrol-applet/index.html: $(srcdir)/webcontrol-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/webcontrol-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/webcontrol-applet
	-cp $(srcdir)/webcontrol-applet/*.html $(distdir)/webcontrol-applet
	-cp $(srcdir)/webcontrol-applet/*.css $(distcidr)/webcontrol-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in webcontrol-applet/*.html webcontrol-applet/*.css; do \
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
