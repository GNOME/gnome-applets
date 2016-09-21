NULL =

invest_applet_libdir = $(pkglibdir)
invest_applet_lib_LTLIBRARIES = libinvest-applet.la

libinvest_applet_la_SOURCES = \
	invest-applet.c \
	invest-applet.h \
	invest-cache.c \
	invest-cache.h \
	invest-chart.c \
	invest-chart.h \
	invest-currencies.c \
	invest-currencies.h \
	invest-image-retriever.c \
	invest-image-retriever.h \
	invest-preferences.c \
	invest-preferences.h \
	invest-quotes.c \
	invest-quotes.h \
	invest-quotes-retriever.c \
	invest-quotes-retriever.h \
	invest-widget.c \
	invest-widget.h \
	invest-window.c \
	invest-window.h \
	$(NULL)

libinvest_applet_la_CPPFLAGS = \
	-DARTDIR=\""$(pkgdatadir)/invest-applet"\" \
	-DBUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DUIDIR=\""$(pkgdatadir)/ui"\" \
	$(NULL)

libinvest_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libinvest_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(NULL)

libinvest_applet_la_LDFLAGS = \
	-module -avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

-include $(top_srcdir)/git.mk
