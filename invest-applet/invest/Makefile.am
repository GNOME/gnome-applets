invest-applet: invest-applet.py
	sed -e "s|\@PYTHONPATH\@|$(pythondir)/invest|" $< > $@

bin_SCRIPTS = invest-chart

investbindir = $(libdir)/invest-applet
investbin_SCRIPTS = invest-applet

investdir = $(pythondir)/invest
invest_PYTHON = \
	__init__.py \
	about.py \
	applet.py \
	chart.py \
	widgets.py \
	quotes.py \
	preferences.py \
	defs.py 
	
BUILT_SOURCES = \
	invest-applet
	
CLEANFILES = \
	$(BUILT_SOURCES)
	
DISTCLEANFILES = \
	defs.py \
	$(CLEANFILES)
	
EXTRA_DIST = \
	defs.py.in \
	invest-applet.py \
	invest-chart
