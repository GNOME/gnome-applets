NULL =

if BUILD_STICKYNOTES_APPLET

libexec_PROGRAMS = stickynotes_applet

stickynotes_applet_CFLAGS = \
	-DSTICKYNOTES_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DPKG_DATA_DIR=\""$(pkgdatadir)"\" \
	-I. \
	-I$(srcdir) \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(XML2_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

stickynotes_applet_SOURCES = \
	gsettings.h \
	stickynotes.c \
	stickynotes.h \
	stickynotes_applet.c \
	stickynotes_applet.h \
	stickynotes_applet_callbacks.c \
	stickynotes_applet_callbacks.h \
	stickynotes_callbacks.c \
	stickynotes_callbacks.h \
	util.c \
	util.h \
	$(NULL)

stickynotes_applet_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

stickynotes_applet_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBWNCK_LIBS) \
	$(X_LIBS) \
	$(XML2_LIBS) \
	$(NULL)

endif

-include $(top_srcdir)/git.mk
