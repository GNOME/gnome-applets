NULL =

SUBDIRS = apmlib

uidir = $(pkgdatadir)/ui

if NEED_LIBAPM
if HAVE_LIBAPM
APMLIB = -lapm
APMDIR =
APMINC =
else
APMLIB = apmlib/libapm.la
APMDIR = apmlib
APMINC = -I$(srcdir)/apmlib/
endif
else
APMLIB =
APMDIR =
APMINC =
endif

libexec_PROGRAMS = battstat-applet-2

battstat_applet_2_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(UPOWER_CFLAGS) \
	$(APMINC) \
	$(ACPIINC) \
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	-DBATTSTAT_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

battstat_applet_2_SOURCES = \
	battstat.h \
	pixmaps.h \
	properties.c \
	battstat_applet.c \
	power-management.c \
	acpi-linux.c \
	acpi-linux.h \
	acpi-freebsd.c \
	acpi-freebsd.h \
	battstat-upower.c \
	battstat-upower.h \
	$(NULL)

battstat_applet_2_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

battstat_applet_2_LDADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(UPOWER_LIBS) \
	$(LIBM) \
	$(APMLIB) \
	$(NULL)

-include $(top_srcdir)/git.mk
