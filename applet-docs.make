# This file gets included by the applets' Makefile.am
# they should set $(applet) to their name, and $(figs)
# to the images they reference
#
# Sample:
#
# applet = modemlights
# lang = C
# figs = modemlights-advpref.png  modemlights-prefs.png  modemlights.png
# include $(top_srcdir)/applet-docs.make

helpdir = $(datadir)/gnome/help/$(applet)_applet/$(lang)
help_DATA =	\
	index.html	\
	topic.dat	\
	$(figs)

# Scrollkeeper related stuff
omf_dir=$(top_srcdir)/omf-install

sgml_files = \
	$(sgml_ents)		\
	$(applet)_applet.sgml

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = $(sgml_files) $(help_DATA) $(omffile)

all: index.html omf

omf: $(omffile)
	-for file in $(omffile); do \
	  scrollkeeper-preinstall $(helpdir)/$(app).sgml $$file $(omf_dir)/$$file; \
	done

index.html: $(applet)_applet/index.html
	-cp $(applet)_applet/index.html .

$(applet)_applet.sgml: $(sgml_ents)
	-ourdir=`pwd`;	\
	cd $(srcdir);	\
	cp $(sgml_ents) $$ourdir

$(applet)_applet/index.html: $(applet)_applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	jw -c /etc/sgml/catalog $$srcdir/$(applet)_applet.sgml -o $$srcdir/$(applet)_applet

applet-dist-hook: index.html
	-$(mkinstalldirs) $(distdir)/$(applet)_applet/stylesheet-images
	-cp $(srcdir)/$(applet)_applet/*.html $(distdir)/$(applet)_applet
	-cp $(srcdir)/$(applet)_applet/*.css  $(distdir)/$(applet)_applet
	-cp $(srcdir)/$(applet)_applet/*.png  $(distdir)/$(applet)_applet
	-cp $(srcdir)/$(applet)_applet/stylesheet-images/*.png \
		$(distdir)/$(applet)_applet/stylesheet-images
	-cp $(srcdir)/$(applet)_applet/stylesheet-images/*.gif \
		$(distdir)/$(applet)_applet/stylesheet-images

install-data-am: index.html omf
	-$(mkinstalldirs) $(DESTDIR)$(helpdir)/stylesheet-images
	-cp $(srcdir)/topic.dat $(DESTDIR)$(helpdir)
	-cp $(srcdir)/$(sgml_files) $(DESTDIR)$(helpdir)
	-for file in \
		$(applet)_applet/*.html	\
		$(applet)_applet/*.css	\
		$(srcdir)/*.png; do\
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(helpdir)/$$basefile; \
	done
	-for file in \
		$(applet)_applet/stylesheet-images/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(helpdir)/stylesheet-images/$$basefile; \
	done
	-for file in \
		$(applet)_applet/stylesheet-images/*.gif; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(helpdir)/stylesheet-images/$$basefile; \
	done

$(applet)_applet.ps: $(srcdir)/$(applet)_applet.sgml $(srcdir)/$(applet).sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2ps $$srcdir/$(applet)_applet.sgml

$(applet)_applet.rtf: $(srcdir)/$(applet)_applet.sgml $(srcdir)/$(applet).sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2ps $$srcdir/$(applet)_applet.sgml

