gnome_applets_helpdir = $(datadir)/gnome/help/gnome-applets/C

gnome_applets_help_DATA = 	\
	index.html		\
	topic.dat

EXTRA_DIST =	\
	config.sgml.in		\
	gnome-applets.sgml	\
	topic.dat		\
	gpl.sgml		\
	gpl2.sgml

config.sgml: config.sgml.in Makefile
	sed -e 's#\@VERSION\@#$(VERSION)#g' \
	    -e 's#\@top-srcdir\@#$(top_srcdir)#g' \
	      < $(srcdir)/config.sgml.in > tmp-t \
	  && mv tmp-t config.sgml

files.sgml: files.sgml.in Makefile
	sed -e 's#\@VERSION\@#$(VERSION)#g' \
	    -e 's#\@top-srcdir\@#$(top_srcdir)#g' \
	      < $(srcdir)/files.sgml.in > tmp-t \
	  && mv tmp-t files.sgml

all: index.html

index.html: gnome-applets/index.html
	-cp gnome-applets/index.html .

# the wierd srcdir trick is because the db2html from the Cygnus RPMs
# cannot handle relative filenames
gnome-applets/index.html: $(srcdir)/gnome-applets.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/gnome-applets.sgml

dist-hook: index.html
	mkdir $(distdir)/gnome-applets
	-cp gnome-applets/*.html $(distdir)/gnome-applets
	-cp gnome-applets/*.css $(distdir)/gnome-applets

install-data-local: index.html
	-for file in $(srcdir)/gnome-applets/*.html $(srcdir)/gnome-applets/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(gnome_applets_helpdir)/$$basefile; \
	done

gnome-applets.ps: gnome-applets.sgml
	-db2ps $<

gnome-applets.rtf: gnome-applets.sgml
	-db2rtf $<
