SUBDIRS = help

EXTRA_DIST = \
	$(server_in_in_files) \
        $(ui_DATA) \
        $(glade_DATA)

libexec_PROGRAMS = gnome-keyboard-applet
noinst_LIBRARIES = libgswitchit-applet.a

gnome_keyboard_applet_SOURCES = switch.c

libgswitchit_applet_a_SOURCES = gswitchit-applet.c \
                         gswitchit-applet.h

if ENABLE_FLAGS
  flags_CFLAGS = -DENABLE_FLAGS=1
endif

COMMON_CFLAGS =				\
	-I$(top_srcdir)			\
	-I.				\
	-DG_LOG_DOMAIN=\"GSwitchIt\"	\
	$(LIBGNOMEKBDUI_CFLAGS)		\
	$(GNOME_APPLETS_CFLAGS)		\
	$(LIBGLADE_CFLAGS)		\
	$(flags_CFLAGS)	

COMMON_LIBS = $(LIBGNOMEKBDUI_LIBS) \
	      $(GNOME_APPLETS_LIBS) \
	      $(LIBGLADE_LIBS) 

libgswitchit_applet_a_CFLAGS = $(COMMON_CFLAGS)

gnome_keyboard_applet_CFLAGS = $(COMMON_CFLAGS)

gnome_keyboard_applet_LDFLAGS = -export-dynamic

gnome_keyboard_applet_LDADD = \
                         libgswitchit-applet.a \
                         $(COMMON_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_KeyboardApplet.server.in
server_in_in_files = $(server_in_files:.server.in=.server.in.in) 
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_in_files) 
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" -e "s|\@VERSION\@|$(VERSION)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA)

@INTLTOOL_SERVER_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_GSwitchItApplet.xml

glade_DATA = gswitchit.glade

