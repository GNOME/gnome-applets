INCLUDES =				\
	-I$(srcdir)			\
	-I$(top_srcdir)			\
	-I$(includedir)			\
	$(GNOME_APPLETS_CFLAGS)		\
	$(GNOME_VFS_APPLETS_CFLAGS)	\
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
