## Process this file with automake to produce Makefile.in

docdir = $(datadir)/gnome/help/screenshooter_applet/C
doc_DATA =	\
	screenshooter.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = screenshooter.sgml topic.dat

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: screenshooter.html

screenshooter.html: screenshooter/screenshooter.html
	-cp screenshooter/*.html .

screenshooter/screenshooter.html: $(srcdir)/screenshooter.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	cat $(srcdir)/../ChangeLog | sed 's/</\&lt;/g; s/>/\&gt;/g' > $$srcdir/change-log.txt ;	\
	db2html $$srcdir/screenshooter.sgml

dist-hook: screenshooter.html
	mkdir $(distdir)/screenshooter
	-cp $(srcdir)/screenshooter/*.html $(distdir)/screenshooter
	-cp $(srcdir)/screenshooter/*.css $(distcidr)/screenshooter

install-data-local: screenshooter.html
	-for file in screenshooter/*.html screenshooter/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
