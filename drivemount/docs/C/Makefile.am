## Process this file with automake to produce Makefile.in

figs = \
	drivemount_applet_all.png     drivemount_applet_settings.png	\
	drivemount_applet_floppy.png  drivemount_applet_tooltip.png

docdir = $(datadir)/gnome/help/drivemount_applet/C
doc_DATA =	\
	index.html	\
	topic.dat	\
	$(figs)

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = drivemount-applet.sgml drivemount.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: drivemount-applet/index.html
	-cp drivemount-applet/index.html .

drivemount-applet/index.html: $(srcdir)/drivemount.sgml $(srcdir)/drivemount-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/drivemount-applet.sgml

dist-hook: index.html
	mkdir $(distdir)/drivemount-applet
	-cp $(srcdir)/drivemount-applet/*.html $(distdir)/drivemount-applet
	-cp $(srcdir)/drivemount-applet/*.css  $(distdir)/drivemount-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in drivemount-applet/*.html drivemount-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

drivemount-applet.ps: drivemount-applet.sgml drivemount.sgml
	-db2ps $<

drivemount-applet.rtf: drivemount-applet.sgml drivemount.sgml
	-db2rtf $<
