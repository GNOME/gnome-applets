## Process this file with automake to produce Makefile.in

figs = \
	battery-applet-ac-offline-25-20.png	\
	battery-applet-properties-messages.png	\
	battery-applet-ac-online-25-20.png	\
	battery-applet-properties-readout.png	\
	battery-applet-graph.png		\
	battery-applet-properties-general.png	\
	battery-applet-properties-graph.png


docdir = $(datadir)/gnome/help/battery_applet/it
doc_DATA =	\
	index.html	\
	topic.dat $(figs)

# automake does not know anything about .sgml files yet -> EXTRA_DIST
EXTRA_DIST = battery-applet.sgml battery.sgml topic.dat $(figs)

# this is taken from gnome-core/gnome-terminal/C/Makefile.am
all: index.html

index.html: battery-applet/index.html
	-cp battery-applet/index.html .

battery-applet/index.html: $(srcdir)/battery.sgml $(srcdir)/battery-applet.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/battery-applet.sgml
dist-hook: index.html
	mkdir $(distdir)/battery-applet
	-cp $(srcdir)/battery-applet/*.html $(distdir)/battery-applet
	-cp $(srcdir)/battery-applet/*.css  $(distdir)/battery-applet

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(docdir)/fig
	-for file in battery-applet/*.html battery-applet/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/$$basefile; \
	done
	-for file in $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/fig/$$basefile; \
	done

battery-applet.ps: battery-applet.sgml battery.sgml
	-db2ps $<

battery-applet.rtf: battery-applet.sgml battery.sgml
	-db2rtf $<
