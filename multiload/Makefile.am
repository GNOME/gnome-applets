# trick Automake into defining $(CXXLINK)
if FALSE
BUILT_SOURCES = dummy.cc

dummy.cc:
	@touch dummy.cc
endif

INCLUDES = -I. -I$(srcdir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
       -I$(srcdir)/../../panel                         \
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = multiload_applet

multiload_applet_SOURCES = cpuload.c 	\
			   cpuload.h	\
			   memload.c	\
			   memload.h	\
			   swapload.c	\
			   swapload.h	\
			   main.c		\
			   linux-proc.h 	\
			   linux-proc.c 	\
			   properties-cpu.c	\
			   properties-cpu.h	\
			   properties-mem.c	\
			   properties-mem.h	\
			   properties-swap.c	\
			   properties-swap.h

multiload_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)      \
		     $(GNOMEUI_LIBS) $(INTLLIBS)	\
		     $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

multiload_applet_LINK = $(CXXLINK)

EXTRA_DIST = Multiload.desktop \
		cpuload_applet.desktop memload_applet.desktop \
		swapload_applet.desktop

appletdir	= $(datadir)/applets

Monitorsdir	= $(appletdir)/Monitors

Multiloaddir	= $(Monitorsdir)/MultiLoad

Multiload_DATA	= cpuload_applet.desktop memload_applet.desktop \
		  swapload_applet.desktop

Directory_files	= Multiload.directory

EXTRA_DIST	= $(Directory_files) $(Multiload_DATA)

install-data-local:
	$(mkinstalldirs) $(Multiloaddir)
	$(INSTALL_DATA) $(srcdir)/Multiload.directory $(Multiloaddir)/.directory
