INCLUDES =	 			\
	-I../../..	 		\
	-I$(srcdir)/../../..	 	\
	-I$(includedir)			\
	$(X_CFLAGS)

lib_LTLIBRARIES = libpanel_cdplayer.la

libpanel_cdplayer_la_SOURCES = 	\
	cdplayer.c led.c cdrom-$(CDROM_HOST).c

EXTRA_libpanel_cdplayer_la_SOURCES = cdrom-linux.c cdrom-solaris.c

dist-hook:
	mkdir $(distdir)/images
	cp $(srcdir)/images/*.xpm $(distdir)/images
