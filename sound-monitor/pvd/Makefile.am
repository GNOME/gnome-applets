INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_builddir)	\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = esdpvd

CFLAGS += $(ESD_CFLAGS)

esdpvd_SOURCES = \
	esdpvd.c	\
	esdpvd.h

esdpvd_LDADD = \
	$(GNOME_LIBDIR)	\
	$(GNOME_LIBS) $(GNOMEUI_LIBS) $(INTLLIBS)
