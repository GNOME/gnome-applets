INCLUDES =  -I$(srcdir)  -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
		$(GNOME_APPLETS_CFLAGS) $(GNOME_VFS_APPLETS_CFLAGS) \
		-I$(top_srcdir) \
		-I$(includedir) \
		$(GAILUTIL_CFLAGS)

noinst_LTLIBRARIES = libgnomedrawareaaccess.la

libgnomedrawareaaccess_la_SOURCES = \
	accessibledrawingarea.c		\
	accessibledrawingarea.h		\
	accessibledrawingareafactory.c		\
	accessibledrawingareafactory.h		\
	customdrawingarea.c		\
	customdrawingarea.h		\
	relate.c		\
	factory.c		\
	factory.h

libgnomedrawareaaccess_la_LIBADD = $(INTLLIBS)
