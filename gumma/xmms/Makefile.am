SUBDIRS = docs

plugindir = $(libdir)/gumma
plugin_LTLIBRARIES = libgumma-xmms.la

INCLUDES = -I. -I$(srcdir) 				\
        -I$(top_srcdir) -I$(top_builddir)               \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

libgumma_xmms_la_LIBADD = \
	$(GNOME_LIBDIR)		\
	$(GNOME_APPLETS_LIBS)	\
	-lxmms

libgumma_xmms_la_SOURCES = \
	xmms.c
