dnl **************************************************************************
dnl Define version info
dnl **************************************************************************

m4_define([ga_major_version], [3])
m4_define([ga_minor_version], [37])
m4_define([ga_micro_version], [0])
m4_define([ga_version], [ga_major_version.ga_minor_version.ga_micro_version])

dnl **************************************************************************
dnl Define bugzilla address
dnl **************************************************************************

m4_define([ga_bugzilla_url],
          [http://bugzilla.gnome.org/enter_bug.cgi?product=gnome-applets])

dnl **************************************************************************
dnl Initialize autoconf
dnl **************************************************************************

AC_INIT([gnome-applets], ga_version, [ga_bugzilla_url])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

dnl **************************************************************************
dnl Set release policy
dnl **************************************************************************

AX_IS_RELEASE([git-directory])

dnl **************************************************************************
dnl Enable debug
dnl **************************************************************************

AX_CHECK_ENABLE_DEBUG([yes])

dnl **************************************************************************
dnl Add warning flags
dnl **************************************************************************

AX_COMPILER_FLAGS([WARN_CFLAGS], [WARN_LDFLAGS])

dnl **************************************************************************
dnl Initialize automake
dnl **************************************************************************

AM_INIT_AUTOMAKE([1.13 no-dist-gzip dist-xz tar-ustar -Wno-portability])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

dnl **************************************************************************
dnl Initialize libtool
dnl **************************************************************************

LT_INIT([dlopen disable-static])
LT_LIB_M

dnl ***************************************************************************
dnl *** Minimum library versions for GNOME Applets                          ***
dnl ***************************************************************************
GTK_REQUIRED=3.20.0
GLIB_REQUIRED=2.44.0
GIO_REQUIRED=2.26.0
LIBPANEL_REQUIRED=3.24.1
LIBGNOME_PANEL_REQUIRED=3.36.0
LIBGTOP_REQUIRED=2.11.92
LIBWNCK_REQUIRED=3.14.1
LIBNOTIFY_REQUIRED=0.7
UPOWER_REQUIRED=0.99.8
ADWAITA_ICON_THEME_REQUIRED=3.14.0
LIBXML_REQUIRED=2.5.0
GWEATHER_REQUIRED=3.28.0
GUCHARMAP3_REQUIRED=2.33.0
POLKIT_REQUIRED=0.97
dnl ***************************************************************************

IT_PROG_INTLTOOL([0.35.0])
PKG_PROG_PKG_CONFIG([0.19])

AC_PATH_PROG([GDBUS_CODEGEN], [gdbus-codegen])
AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources], [no])
if test x$GLIB_COMPILE_RESOURCES = xno; then
  AC_MSG_ERROR([Could not find a glib-compile-resources in your PATH])
fi

AC_PROG_CC
AC_ISC_POSIX
AC_PATH_XTRA

X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
AC_SUBST(X_LIBS)

dnl ***************************************************************************
dnl *** Use pkg-config to check for dependancies                            ***
dnl ***************************************************************************

dnl -- Check for adwaita-icon-theme (required) ----------------------------------
PKG_CHECK_MODULES(ICON_THEME, adwaita-icon-theme >= $ADWAITA_ICON_THEME_REQUIRED)

dnl -- check for gio (required) ------------------------------------------
PKG_CHECK_MODULES(GIO, gio-2.0 >= $GIO_REQUIRED)

PKG_CHECK_MODULES(GIO_UNIX, gio-unix-2.0 >= $GIO_REQUIRED)

dnl -- Check for GTK+ 3.0 (required) ------------------------------------------
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED)

dnl -- check for libpanelapplet (required) ------------------------------------
PKG_CHECK_MODULES(GNOME_APPLETS, libpanel-applet >= $LIBPANEL_REQUIRED)

AC_ARG_WITH([libpanel-applet-dir], [], [LIBPANEL_APPLET_DIR=$withval], [LIBPANEL_APPLET_DIR=""])
if test "$LIBPANEL_APPLET_DIR" == ""; then
	LIBPANEL_APPLET_DIR=`$PKG_CONFIG --variable=libpanel_applet_dir libpanel-applet`
fi

AC_MSG_NOTICE([installing applets in $LIBPANEL_APPLET_DIR])
AC_SUBST(LIBPANEL_APPLET_DIR)

dnl -- check for libgnome-panel (required) ------------------------------------
PKG_CHECK_MODULES([GNOME_PANEL], [libgnome-panel >= $LIBGNOME_PANEL_REQUIRED])

GNOME_PANEL_MODULES_DIR=`$PKG_CONFIG --variable=modulesdir libgnome-panel`
AC_SUBST([GNOME_PANEL_MODULES_DIR], [$GNOME_PANEL_MODULES_DIR])

dnl -- check for X11 (required) -----------------------------------------------
PKG_CHECK_MODULES([X11], [x11])

dnl -- check for libgtop (required) -------------------------------------------
PKG_CHECK_MODULES(GTOP_APPLETS, [libgtop-2.0 >= $LIBGTOP_REQUIRED])

dnl -- check for libwnck (required) -------------------------------------------
PKG_CHECK_MODULES(LIBWNCK, libwnck-3.0 >= $LIBWNCK_REQUIRED,,
	AC_MSG_ERROR([libwnck is required to build gnome-applets.], 1))

dnl -- check for libnotify (required) -----------------------------------------
PKG_CHECK_MODULES(LIBNOTIFY, libnotify >= $LIBNOTIFY_REQUIRED)

PKG_CHECK_MODULES([CPUFREQ_SELECTOR], [
  gio-unix-2.0 >= $GIO_REQUIRED)
  glib-2.0 >= $GLIB_REQUIRED
  gobject-2.0 >= $GLIB_REQUIRED
  polkit-gobject-1 >= $POLKIT_REQUIRED
])

dnl -- check for libupower-glib (required) --------------------------------------------
PKG_CHECK_MODULES(UPOWER, upower-glib >= $UPOWER_REQUIRED)

dnl -- check for wirelesstools (optional) -------------------------------------

ac_have_iwevgenie=no
AC_MSG_CHECKING([for wireless-tools >= 28pre9])


AC_COMPILE_IFELSE([AC_LANG_SOURCE([
	#include <iwlib.h>
	#ifndef IWEVGENIE
	#error "not found"
	#endif
])], [ac_have_iwevgenie=yes])
AC_MSG_RESULT([$ac_have_iwevgenie])
if test "$ac_have_iwevgenie" = yes; then
         AC_DEFINE(HAVE_IW, 1, [Defined if you have wirelesstools])
	IWLIB=-liw
	AC_SUBST(IWLIB)
fi

dnl -- check for gucharmap (optional) -----------------------------------------

PKG_CHECK_MODULES([GUCHARMAP],[gucharmap-2.90 >= $GUCHARMAP3_REQUIRED],
                  [have_gucharmap=yes],[have_gucharmap=no])
if test "$have_gucharmap" = "yes"; then
	AC_DEFINE([HAVE_GUCHARMAP],[1],[Gucharmap Available])
else
  AC_MSG_WARN([*** 'charpick' applet will not be built with gucharmap support ***])
fi

dnl -- check for libxml2 (required) ------------------------------------------
PKG_CHECK_MODULES(XML2, libxml-2.0 >= $LIBXML_REQUIRED)

 dnl -- check for libgweather (required) ------------------
PKG_CHECK_MODULES(LIBGWEATHER, gweather-3.0 >= $GWEATHER_REQUIRED)

 dnl -- check for tracker-sparql (required for tracker-search-bar) ------------
TRACKER_SEARCH_BAR_REQUIRED="tracker-sparql-2.0"

PKG_CHECK_MODULES(TRACKER_SEARCH_BAR, [$TRACKER_SEARCH_BAR_REQUIRED],
                  [have_tracker_search_bar=yes],
                  [have_tracker_search_bar=no])

dnl -- check for GSettings (required for gweather applet)
GLIB_GSETTINGS

dnl ***************************************************************************
dnl *** Set up gettext                                                      ***
dnl ***************************************************************************

GETTEXT_PACKAGE=gnome-applets-3.0
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext Package])
AM_GLIB_GNU_GETTEXT

YELP_HELP_INIT

dnl ***************************************************************************
dnl *** keyboard accessibility status applet check                          ***
dnl ***************************************************************************

savecppflags=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $X_CFLAGS"
AC_CHECK_HEADER(X11/extensions/XKB.h, HAVE_XKB=true, HAVE_XKB=false)
AC_SUBST(HAVE_XKB)

AM_CONDITIONAL(APPLET_ACCESSX, $HAVE_XKB)
if test "x$APPLET_ACCESSX" = "xtrue"; then
  AC_MSG_WARN([*** keyboard accessibility status applet will not be built ***])
fi
CPPFLAGS=$savecppflags

dnl ***************************************************************************
dnl *** cpufreq applet check                                                ***
dnl ***************************************************************************

dnl --disable-cpufreq=(yes|no)
AC_ARG_ENABLE(cpufreq,
	      [  --disable-cpufreq[@<:@=no/yes@:>@]
                          don't build cpu frequency scaling monitor applet
			  [@<:@default: no@:>@]],,
	      disable_cpufreq=no)

dnl --enable-frequency-selector=(yes/no)
AC_ARG_ENABLE(frequency-selector,
	      [  --enable-frequency-selector[@<:@=yes/no@:>@]
	                  build frequency selector [@<:@default: yes@:>@]], enable_selector=$enableval,
	      enable_selector=yes)

build_cpufreq_applet=yes
AS_IF([test "x$disable_cpufreq" = "xno"], [
  case "${host}" in
    *linux*)
      AC_CHECK_HEADER([cpufreq.h], [
        AC_CHECK_LIB([cpupower], [cpupower_is_cpu_online], [
          AC_DEFINE([HAVE_IS_CPU_ONLINE], 1,
                    [Define to 1 if cpupower_is_cpu_online() is available])
          cpufreq_lib="cpupower"

          AC_CHECK_LIB([cpupower], [cpufreq_get_frequencies], [
            AC_DEFINE([HAVE_GET_FREQUENCIES], 1,
                      [Define to 1 if cpufreq_get_frequencies() is available])
          ])
        ], [
          AC_CHECK_LIB([cpupower], [cpufreq_cpu_exists], [
            cpufreq_lib="cpupower"
          ], [
            AC_CHECK_LIB([cpufreq], [cpufreq_cpu_exists], [
              cpufreq_lib="cpufreq"
            ], [cpufreq_lib=])
          ])
        ])

        AS_IF([test "x$cpufreq_lib" != "x"], [
          LIBCPUFREQ_LIBS="-l$cpufreq_lib"
          AC_SUBST([LIBCPUFREQ_LIBS])
        ], [
          AC_MSG_WARN([*** cpufreq applet will not be built ***])
          build_cpufreq_applet=no
        ])
      ], [
        AC_MSG_WARN([*** can't find cpufreq.h, cpufreq applet will not be built ***])
        build_cpufreq_applet=no
      ])
      ;;
    *)
      AC_MSG_WARN([${host} is not supported by cpufreq applet, not building])
      build_cpufreq_applet=no
      ;;
  esac
], [build_cpufreq_applet=no])

AM_CONDITIONAL(BUILD_CPUFREQ_APPLET, test x$build_cpufreq_applet = xyes)
AM_CONDITIONAL(BUILD_CPUFREQ_SELECTOR, test x$enable_selector = xyes)

dnl ***************************************************************************
dnl *** tracker-search-bar applet check                                     ***
dnl ***************************************************************************

AC_ARG_ENABLE([tracker-search-bar],
              AS_HELP_STRING([--enable-tracker-search-bar],
                             [enable tracker-search-bar[[default=auto]]]),,
              [enable_tracker_search_bar=auto])

if test "x$enable_tracker_search_bar" = "xyes" ; then
   if test "x$have_tracker_search_bar" != "xyes"; then
      AC_MSG_ERROR([Couldn't find tracker-search-bar dependencies ($TRACKER_SEARCH_BAR_REQUIRED).])
   fi
else
   if test "x$enable_tracker_search_bar" = "xno"; then
      have_tracker_search_bar="no  (disabled)"
   fi
fi

AM_CONDITIONAL(HAVE_TRACKER_SEARCH_BAR, test "$have_tracker_search_bar" = "yes")

dnl **************************************************************************
dnl Process .in files
dnl **************************************************************************

AC_CONFIG_FILES([
  Makefile

  data/Makefile
  data/icons/Makefile
  data/icons/16x16/Makefile
  data/icons/16x16/apps/Makefile
  data/icons/16x16/devices/Makefile
  data/icons/16x16/status/Makefile
  data/icons/22x22/Makefile
  data/icons/22x22/apps/Makefile
  data/icons/22x22/status/Makefile
  data/icons/24x24/Makefile
  data/icons/24x24/apps/Makefile
  data/icons/24x24/status/Makefile
  data/icons/32x32/Makefile
  data/icons/32x32/apps/Makefile
  data/icons/32x32/status/Makefile
  data/icons/48x48/Makefile
  data/icons/48x48/apps/Makefile
  data/icons/48x48/status/Makefile
  data/icons/72x72/Makefile
  data/icons/72x72/apps/Makefile
  data/icons/96x96/Makefile
  data/icons/96x96/apps/Makefile
  data/icons/scalable/Makefile
  data/icons/scalable/apps/Makefile
  data/icons/scalable/status/Makefile
  data/schemas/Makefile

  gnome-applets/Makefile
  gnome-applets/timer/Makefile
  gnome-applets/window-picker/Makefile

  help/Makefile
  help/accessx-status/Makefile
  help/battstat/Makefile
  help/charpick/Makefile
  help/cpufreq/Makefile
  help/drivemount/Makefile
  help/geyes/Makefile
  help/gweather/Makefile
  help/mini-commander/Makefile
  help/multiload/Makefile
  help/netspeed/Makefile
  help/sticky-notes/Makefile
  help/trash/Makefile

  po/Makefile.in

  accessx-status/Makefile
  accessx-status/pixmaps/Makefile
  accessx-status/src/Makefile

  battstat/Makefile
  battstat/sounds/Makefile
  battstat/src/Makefile

  brightness/Makefile
  brightness/src/Makefile

  charpick/Makefile
  charpick/src/Makefile

  command/Makefile
  command/src/Makefile

  cpufreq/Makefile
  cpufreq/pixmaps/Makefile
  cpufreq/src/cpufreq-selector/Makefile
  cpufreq/src/Makefile

  drivemount/Makefile
  drivemount/src/Makefile

  inhibit/Makefile
  inhibit/src/Makefile

  geyes/Makefile
  geyes/src/Makefile
  geyes/themes/Makefile

  gweather/Makefile
  gweather/src/Makefile

  mini-commander/Makefile
  mini-commander/src/Makefile

  multiload/Makefile
  multiload/src/Makefile

  netspeed/data/Makefile
  netspeed/src/Makefile
  netspeed/Makefile

  sticky-notes/Makefile
  sticky-notes/data/Makefile
  sticky-notes/src/Makefile
  sticky-notes/pixmaps/Makefile

  tracker-search-bar/Makefile
  tracker-search-bar/src/Makefile
  tracker-search-bar/data/Makefile

  trash/Makefile
  trash/src/Makefile
  trash/data/Makefile

  window-buttons/Makefile
  window-buttons/themes/Makefile

  window-title/Makefile
])

AC_OUTPUT

dnl **************************************************************************
dnl Show summary
dnl **************************************************************************

echo ""
echo "  GNOME Applets $VERSION"
echo "  `echo GNOME Applets $VERSION | sed "s/./=/g"`"
echo ""
echo "  srcdir ..........................: ${srcdir}"
echo ""
echo "  compiler ........................: ${CC}"
echo "  cflags ..........................: ${CFLAGS}"
echo ""
echo "  prefix ..........................: ${prefix}"
echo "  exec_prefix .....................: ${exec_prefix}"
echo "  libdir ..........................: ${libdir}"
echo "  libexecdir ......................: ${libexecdir}"
echo "  sysconfdir ......................: ${sysconfdir}"
echo ""
echo "  libgnome-panel"
echo "    modulesdir ....................: ${GNOME_PANEL_MODULES_DIR}"
echo ""
echo "  Applets"
echo "    accessx-status ................: $HAVE_XKB"
echo "    batstat .......................: always"
echo "    brightness ....................: always"
echo "    charpick ......................: always"
echo "    command .......................: always"
echo "    cpufreq .......................: $build_cpufreq_applet"
echo "    drivemount ....................: always"
echo "    geyes .........................: always"
echo "    gweather ......................: always"
echo "    inhibit .......................: always"
echo "    mini-commander ................: always"
echo "    multiload .....................: always"
echo "    netspeed ......................: always"
echo "    sticky-notes ..................: always"
echo "    timer .........................: always"
echo "    tracker-search-bar ............: $have_tracker_search_bar"
echo "    trash .........................: always"
echo "    window-buttons ................: always"
echo "    window-picker .................: always"
echo "    window-title ..................: always"
echo ""
echo "  charpick"
echo "    gucharmap support .............: $have_gucharmap"
echo ""
echo "  cpufreq"
echo "    building selector .............: $enable_selector"
echo ""
