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

battery_status_applet_libdir = $(pkglibdir)/$(LIBPANEL_APPLET_API_VERSION)
battery_status_applet_lib_LTLIBRARIES = libbattery-status-applet.la

libbattery_status_applet_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	-DBATTSTAT_MENU_UI_DIR=\""$(uidir)"\" \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DGTK_BUILDERDIR=\""$(pkgdatadir)/builder"\" \
	-DG_LOG_DOMAIN=\"org.gnome.gnome-applets.battery-status\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(NULL)

libbattery_status_applet_la_CFLAGS = \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(UPOWER_CFLAGS) \
	$(APMINC) \
	$(ACPIINC) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libbattery_status_applet_la_SOURCES = \
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

libbattery_status_applet_la_LDFLAGS = \
	-avoid-version \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

libbattery_status_applet_la_LIBADD = \
	$(GNOME_APPLETS_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(UPOWER_LIBS) \
	$(LIBM) \
	$(APMLIB) \
	$(NULL)

-include $(top_srcdir)/git.mk
