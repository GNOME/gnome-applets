## Process this file with automake to produce Makefile.in

docdir = $(datadir)/gnome/help/diskusage_applet/C
doc_DATA =	\
	index.html	\
	topic.dat	\
	diskusage-applet-prefs.png  diskusage-applet.png

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = diskusage-applet.sgml diskusage.sgml topic.dat	\
	diskusage-applet-prefs.png  diskusage-applet.png

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: diskusage-applet/index.html
	-cp diskusage-applet/index.html .

diskusage-applet/index.html: $(srcdir)/diskusage-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/diskusage-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/diskusage-applet
	-cp $(srcdir)/diskusage-applet/*.html $(distdir)/diskusage-applet
	-cp $(srcdir)/diskusage-applet/*.css $(distcidr)/diskusage-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in diskusage-applet/*.html diskusage-applet/*.css; do \
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
