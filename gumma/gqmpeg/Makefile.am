plugindir = $(libdir)/gumma
plugin_LTLIBRARIES = libgumma-gqmpeg.la

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

libgumma_gqmpeg_la_LIBADD = \
	$(GNOME_LIBDIR)		\
	$(GNOME_APPLETS_LIBS)	\
	$(INTLLIBS)

libgumma_gqmpeg_la_SOURCES = \
	gqmpeg.c
