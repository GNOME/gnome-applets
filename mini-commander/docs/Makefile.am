## Process this file with automake to produce Makefile.in

docdir = $(datadir)/gnome/help/mini-commander_applet/C
doc_DATA =	\
	index.html	\
	topic.dat

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = mini-commander.sgml gpl2.sgml topic.dat

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: mini-commander/index.html
	-cp mini-commander/index.html .

mini-commander/index.html: $(srcdir)/mini-commander.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	cat $(srcdir)/../ChangeLog | sed 's/</\&lt;/g; s/>/\&gt;/g' > $$srcdir/change-log.txt ;	\
	cat $(srcdir)/../TODO | sed 's/</\&lt;/g; s/>/\&gt;/g' > $$srcdir/todo.txt ;	\
	cat $(srcdir)/../NEWS | sed 's/</\&lt;/g; s/>/\&gt;/g' > $$srcdir/news.txt ;	\
	db2html $$srcdir/mini-commander.sgml

dist-hook: index.html
	mkdir $(distdir)/mini-commander
	-cp $(srcdir)/mini-commander/*.html $(distdir)/mini-commander
	-cp $(srcdir)/mini-commander/*.css $(distcidr)/mini-commander

install-data-local: index.html
	-for file in mini-commander/*.html mini-commander/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done

gnome-terminal.ps: gnome-terminal.sgml
	-db2ps $<

gnome-terminal.rtf: gnome-terminal.sgml
	-db2rtf $<