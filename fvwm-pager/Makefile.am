Utilitiesdir = $(datadir)/apps/Utilities

INCLUDES =                                              \
	-I. -I$(srcdir)                                 \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../../panel				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)             \
	@FVWM2INCLUDE@


bin_PROGRAMS = fvwm-pager

fvwm_pager_SOURCES = fvwm-pager.c \
                     gtkpager.c \
                     gtkpager.h  \
		     pager-win.h \
		     pager-win.c \
		     properties.h \
		     properties.c

fvwm_pager_LDADD = \
	../../panel/libpanel_applet.la \
	@FVWM2LIBS@                    \
	$(GNOME_LIBDIR) \
	$(GNOMEGNORBA_LIBS)                \
	$(INTLLIBS)

CFLAGS += -DVERSION=\"0.2\"

EXTRA_DIST = TODO README NEWS fvwm_pager.gnorba
gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = fvwm_pager.gnorba 
