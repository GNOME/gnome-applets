EXTRA_DIST =	\
	config.sgml.in		\
	gnome-applets.sgml	\
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

