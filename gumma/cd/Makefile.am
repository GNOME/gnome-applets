plugindir = $(libdir)/gumma
plugin_LTLIBRARIES = libgumma-cd.la

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

libgumma_cd_la_LIBADD = \
	$(GNOME_LIBDIR)		\
	$(GNOME_APPLETS_LIBS)	\
	$(INTLLIBS)

libgumma_cd_la_SOURCES = \
	cdplayer.c		\
	cdrom-$(CDROM_HOST).c	\
	cdrom-interface.h

EXTRA_DIST = \
	cdrom-linux.c	\
	cdrom-solaris.c
