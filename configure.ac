dnl **************************************************************************
dnl Define version info
dnl **************************************************************************

m4_define([ga_major_version], [3])
m4_define([ga_minor_version], [20])
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

AC_INIT([gnome-applets], [ga_version], [ga_bugzilla_url])
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
dnl *** Minimum library versions for GNOME-APPLETS                          ***
dnl ***************************************************************************
GTK_REQUIRED=3.20.0
GLIB_REQUIRED=2.44.0
GIO_REQUIRED=2.26.0
LIBPANEL_REQUIRED=3.22.0
LIBGTOP_REQUIRED=2.11.92
LIBWNCK_REQUIRED=3.14.1
LIBNOTIFY_REQUIRED=0.7
UPOWER_REQUIRED=0.9.4
DBUS_REQUIRED=1.1.2
DBUS_GLIB_REQUIRED=0.74
ADWAITA_ICON_THEME_REQUIRED=3.14.0
LIBXML_REQUIRED=2.5.0
GWEATHER_REQUIRED=3.17.1
GUCHARMAP3_REQUIRED=2.33.0
POLKIT_REQUIRED=0.92
dnl ***************************************************************************

IT_PROG_INTLTOOL([0.35.0])
PKG_PROG_PKG_CONFIG([0.19])

AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources], [no])
if test x$GLIB_COMPILE_RESOURCES = xno; then
  AC_MSG_ERROR([Could not find a glib-compile-resources in your PATH])
fi

AC_PROG_CC
AC_ISC_POSIX
AC_STDC_HEADERS
AC_PATH_XTRA

X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
AC_SUBST(X_LIBS)

dnl Check for non-portable headers
AC_CHECK_HEADERS([pty.h values.h])

dnl ***************************************************************************
dnl *** Use pkg-config to check for dependancies                            ***
dnl ***************************************************************************

dnl -- Check for adwaita-icon-theme (required) ----------------------------------
PKG_CHECK_MODULES(ICON_THEME, adwaita-icon-theme >= $ADWAITA_ICON_THEME_REQUIRED)

dnl -- check for gio (required) ------------------------------------------
PKG_CHECK_MODULES(GIO, gio-2.0 >= $GIO_REQUIRED)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)

PKG_CHECK_MODULES(GIO_UNIX, gio-unix-2.0 >= $GIO_REQUIRED)
AC_SUBST(GIO_UNIX_CFLAGS)
AC_SUBST(GIO_UNIX_LIBS)

dnl -- Check for GTK+ 3.0 (required) ------------------------------------------
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED)
AC_SUBST(GTK_FLAGS)
AC_SUBST(GTK_LIBS)

dnl -- check for libpanelapplet (required) ------------------------------------
PKG_CHECK_MODULES(GNOME_APPLETS, libpanel-applet >= $LIBPANEL_REQUIRED)
AC_SUBST(GNOME_APPLETS_CFLAGS)
AC_SUBST(GNOME_APPLETS_LIBS)

AC_ARG_WITH([libpanel-applet-dir], [], [LIBPANEL_APPLET_DIR=$withval], [LIBPANEL_APPLET_DIR=""])
if test "$LIBPANEL_APPLET_DIR" == ""; then
	LIBPANEL_APPLET_DIR=`$PKG_CONFIG --variable=libpanel_applet_dir libpanel-applet`
fi

AC_MSG_NOTICE([installing applets in $LIBPANEL_APPLET_DIR])
AC_SUBST(LIBPANEL_APPLET_DIR)

dnl -- check for libgtop (optional) -------------------------------------------
build_gtop_applets=false
PKG_CHECK_MODULES(GTOP_APPLETS,
                 [libgtop-2.0 >= $LIBGTOP_REQUIRED],
                 build_gtop_applets=true,
		 AC_MSG_WARN([libgtop not found. Not building gtop applets.]))
AC_SUBST(GTOP_APPLETS_CFLAGS)
AC_SUBST(GTOP_APPLETS_LIBS)
AM_CONDITIONAL(BUILD_GTOP_APPLETS, $build_gtop_applets)

dnl -- check for gnome-settings-daemon (optional) -----------------------------
PKG_CHECK_MODULES(GNOME_SETTINGS_DAEMON, gnome-settings-daemon,
	HAVE_GNOME_SETTINGS_DAEMON=yes, HAVE_GNOME_SETTINGS_DAEMON=no)

if test "x$HAVE_GNOME_SETTINGS_DAEMON" = "xno"; then
	AC_MSG_WARN([gnome-settings-daemon DBUS API is required to build modem-applet.])
fi

AC_SUBST(GNOME_SETTINGS_DAEMON_CFLAGS)
AC_SUBST(GNOME_SETTINGS_DAEMON_LIBS)

dnl -- check for libwnck (required) -------------------------------------------
PKG_CHECK_MODULES(LIBWNCK, libwnck-3.0 >= $LIBWNCK_REQUIRED,,
	AC_MSG_ERROR([libwnck is required to build gnome-applets.], 1))

AC_SUBST(LIBWNCK_CFLAGS)
AC_SUBST(LIBWNCK_LIBS)

dnl -- check for libnotify (optional) -----------------------------------------
LIBNOTIFY_CFLAGS=
LIBNOTIFY_LIBS=
PKG_CHECK_MODULES(LIBNOTIFY, libnotify >= $LIBNOTIFY_REQUIRED,
		  HAVE_LIBNOTIFY="yes", HAVE_LIBNOTIFY="no")

if test "x$HAVE_LIBNOTIFY" = "xyes"; then
	AC_DEFINE(HAVE_LIBNOTIFY, 1, [libnotify available])
fi

AC_SUBST(LIBNOTIFY_CFLAGS)
AC_SUBST(LIBNOTIFY_LIBS)

dnl -- check for glib/gobject (required) --------------------------------------
PKG_CHECK_MODULES(CPUFREQ_SELECTOR, glib-2.0 >= $GLIB_REQUIRED gobject-2.0 >= $GLIB_REQUIRED)
AC_SUBST(CPUFREQ_SELECTOR_CFLAGS)
AC_SUBST(CPUFREQ_SELECTOR_LIBS)

dnl -- check for dbus (required for cpufreq-selector)
PKG_CHECK_MODULES(DBUS, dbus-1 >= $DBUS_REQUIRED dbus-glib-1 >= $DBUS_GLIB_REQUIRED, HAVE_DBUS=yes, HAVE_DBUS=no)

AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

dnl -- check for PolicyKit and PolicyKit-gnome (optional) -----------------------------------------
POLKIT_CFLAGS=
POLKIT_LIBS=
AC_ARG_ENABLE([polkit],
    AC_HELP_STRING([--enable-polkit], [Enable PolicyKit support @<:@default auto@:>@]),
    enable_polkit=$enableval,
    enable_polkit=auto)
if test "x$enable_polkit" != "xno"; then
    PKG_CHECK_MODULES(POLKIT, polkit-gobject-1 >= $POLKIT_REQUIRED dbus-glib-1 >= $DBUS_GLIB_REQUIRED, HAVE_POLKIT=yes, HAVE_POLKIT=no)
    if test "x$enable_polkit" = "xyes" -a "x$HAVE_POLKIT" = "xno"; then
        AC_MSG_ERROR([PolicyKit support explicitly requested but dependencies not found])
    fi
fi
dnl - This is only so the printout at the end of configure is correct -
if test "x$enable_polkit" = "xno"; then
   HAVE_POLKIT=no
fi
if test "x$HAVE_POLKIT" = "xyes"; then
    AC_DEFINE(HAVE_POLKIT, [1], [PolicyKit available])
fi
AM_CONDITIONAL(HAVE_POLKIT, test "x$HAVE_POLKIT" = "xyes")
AC_SUBST(POLKIT_CFLAGS)
AC_SUBST(POLKIT_LIBS)

dnl -- check for libupower-glib (optional) --------------------------------------------
UPOWER_CFLAGS=
UPOWER_LIBS=
AC_ARG_WITH(upower,[  --without-upower           build without upower support])

if test "x$with_upower" != xno; then
	PKG_CHECK_MODULES(UPOWER, upower-glib >= $UPOWER_REQUIRED,
		  HAVE_UPOWER="yes",
		  HAVE_UPOWER="no")

	if test "x$HAVE_UPOWER" = "xyes"; then
		AC_DEFINE(HAVE_UPOWER, 1, [UPOWER available])
	fi
else
	AC_MSG_WARN(["upower support disabled"])
fi

AC_SUBST(UPOWER_CFLAGS)
AC_SUBST(UPOWER_LIBS)
AC_SUBST(LIBM)

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
AC_SUBST(GUCHARMAP_CFLAGS)
AC_SUBST(GUCHARMAP_LIBS)

dnl -- check for libxml2 (required) ------------------------------------------
PKG_CHECK_MODULES(XML2, libxml-2.0 >= $LIBXML_REQUIRED)
AC_SUBST(XML2_CFLAGS)
AC_SUBST(XML2_LIBS)

 dnl -- check for libgweather (required for gweather applet) ------------------
build_libgweather_applets=false
PKG_CHECK_MODULES(LIBGWEATHER, gweather-3.0 >= $GWEATHER_REQUIRED,
	          build_libgweather_applets=true,
		  AC_MSG_WARN([libgweather not found. Not building the weather applet.]))
AC_SUBST(LIBGWEATHER_CFLAGS)
AC_SUBST(LIBGWEATHER_LIBS)
AM_CONDITIONAL(BUILD_LIBGWEATHER_APPLETS, $build_libgweather_applets)

 dnl -- check for tracker-sparql (required for tracker-search-bar) ------------
TRACKER_SEARCH_BAR_REQUIRED="tracker-sparql-1.0"

PKG_CHECK_MODULES(TRACKER_SEARCH_BAR, [$TRACKER_SEARCH_BAR_REQUIRED],
                  [have_tracker_search_bar=yes],
                  [have_tracker_search_bar=no])
AC_SUBST(TRACKER_SEARCH_BAR_CFLAGS)
AC_SUBST(TRACKER_SEARCH_BAR_LIBS)

dnl -- check for GSettings (required for gweather applet)
GLIB_GSETTINGS

dnl ***************************************************************************
dnl *** find paths to installed binaries                                    ***
dnl ***************************************************************************

dnl -- gdk-pixbuf-csource -----------------------------------------------------
AC_PATH_PROG(GDK_PIXBUF_CSOURCE, gdk-pixbuf-csource, no)

if test x"$GDK_PIXBUF_CSOURCE" = xno; then
  AC_MSG_ERROR([gdk-pixbuf-csource executable not found in your path - should be installed with GTK])
fi
AC_SUBST(GDK_PIXBUF_CSOURCE)

dnl -- glib-genmarshal --------------------------------------------------------
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

dnl -- xsltproc ---------------------------------------------------------------
AC_PATH_PROG(XSLTPROC, xsltproc, no)
if test x"$XSLTPROC" = xno; then
  AC_MSG_ERROR([xsltproc executable not found in your path - should be installed with libxslt])
fi

dnl -- jw ---------------------------------------------------------------------
AC_PATH_PROG(JW, jw, no)
if test x$JW = xno; then
  HAVE_JW="no"
else
  HAVE_JW="yes"
fi
AM_CONDITIONAL(HAVE_JW, test "x$HAVE_JW" = "xyes")
AC_SUBST(HAVE_JW)

dnl ***************************************************************************
dnl *** Set up gettext                                                      ***
dnl ***************************************************************************

GETTEXT_PACKAGE=gnome-applets-3.0
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext Package])
AM_GLIB_GNU_GETTEXT

YELP_HELP_INIT

case "$host" in
	sparc-*-solaris*)	gkb_type_sun=true;;
	*)			gkb_type_sun=false;;
esac
AM_CONDITIONAL(GKB_SUN, $gkb_type_sun)

dnl ***************************************************************************
dnl *** battstat specific checks                                            ***
dnl ***************************************************************************

dnl --disable-battstat=(yes|no)
AC_ARG_ENABLE(battstat,
	      [  --disable-battstat=[@<:@no/yes@:>@]
                          don't build battery monitor applet [@<:@default: no@:>@]],,
	      disable_battstat=no)

HAVE_LIBAPM=no
NEED_LIBAPM=no
ACPIINC=
build_battstat_applet=no

if test x$disable_battstat = xno; then
   build_battstat_applet=yes
   AC_CHECK_HEADERS([err.h sys/sysctl.h])
   case "${host}" in
       powerpc-*-linux*)
           NEED_LIBAPM=yes
           AC_CHECK_LIB(apm,apm_read,[HAVE_LIBAPM=yes],[HAVE_LIBAPM=no])
	   ;;
       # list of supported OS cores that use libapm
       *-*-linux*)
	   NEED_LIBAPM=yes
	   AC_CHECK_LIB(apm,apm_read,[HAVE_LIBAPM=yes],[HAVE_LIBAPM=no])
	   ;;
       # list of supported OS cores that do not use libapm
       i386-*-freebsd*|*-*-netbsd*|*-*-openbsd*|*-*kfreebsd*-gnu)
          if [ -n "${OS_SYS}" ]; then
	       ACPIINC="-I${OS_SYS}"
	   else
	       ACPIINC="-I/usr/src/sys"
	   fi

	   savecppflags=$CPPFLAGS
	   CPPFLAGS="$CPPFLAGS $ACPIINC"
	   AC_CHECK_HEADER(dev/acpica/acpiio.h, have_acpiio=true, have_acpiio=false)
	   if test $have_acpiio = true; then
	     AC_DEFINE(HAVE_ACPIIO, 1, [Define if acpiio.h exists on the system])
	   fi

	   ;;
       *)
	   echo "warning: ${host} is not supported by battstat_applet, not building" >&2
	   build_battstat_applet=no
	   ;;
    esac
fi

AC_SUBST(HAVE_LIBAPM)
AC_SUBST(NEED_LIBAPM)
AC_SUBST(ACPIINC)
AM_CONDITIONAL(HAVE_LIBAPM, test "x$HAVE_LIBAPM" = "xyes")
AM_CONDITIONAL(NEED_LIBAPM, test "x$NEED_LIBAPM" = "xyes")
AM_CONDITIONAL(BUILD_BATTSTAT_APPLET, test x$build_battstat_applet = xyes)

dnl ***************************************************************************
dnl *** Mini-Commander specific checks                                      ***
dnl ***************************************************************************
AC_ARG_ENABLE(mini-commander, [  --enable-mini-commander build mini-commander
                          [@<:@default: no, transparent upgrade to deskbar-applet@:>@]], enable_mini_commander="yes", enable_mini_commander="no")
AM_CONDITIONAL(BUILD_MINICOMMANDER_APPLET, test "x$enable_mini_commander" = "xyes")

dnl ***************************************************************************
dnl *** Stickynotes specific checks                                         ***
dnl ***************************************************************************
# AC_ARG_ENABLE(stickynotes, [  --enable-stickynotes build stickynotes
#                           [@<:@default: no, transparent upgrade to Tomboy@:>@]], enable_stickynotes="yes", enable_stickynotes="no")
enable_stickynotes="yes"
AM_CONDITIONAL(BUILD_STICKYNOTES_APPLET, test "x$enable_stickynotes" = "xyes")

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
dnl *** modemlights applet check                                            ***
dnl ***************************************************************************

SU_TOOL=
HAVE_LIBUTIL=

AC_PATH_PROG(SU_TOOL, su)
AC_CHECK_LIB(util, forkpty, HAVE_LIBUTIL=true, HAVE_LIBUTIL=false)

if test "x$HAVE_LIBUTIL" = "xtrue" -a "x$HAVE_GNOME_SETTINGS_DAEMON" = "xyes"; then
  MODEMLIGHTS_LIBS=" -lutil"
  AC_SUBST(MODEMLIGHTS_LIBS)
  AC_DEFINE_UNQUOTED(STB_SCRIPTS_DIR, "$STB_SCRIPTS_DIR", [System tools backends])
  AC_DEFINE_UNQUOTED(SU_PATH, "${SU_TOOL}", [su executable path])
  BUILD_MODEM_LIGHTS=yes
else
  AC_MSG_WARN([*** modemlights applet will not be built ***])
  BUILD_MODEM_LIGHTS=no
fi

AM_CONDITIONAL(APPLET_MODEMLIGHTS, test "x$BUILD_MODEM_LIGHTS" = "xyes")

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

dnl --enable-suid=(yes/no)
AC_ARG_ENABLE(suid,
       [  --enable-suid[@<:@=yes/no@:>@]  suid root the frequency selector executable
                   [@<:@default: yes@:>@]], ,
       suid=yes)

if test "x$HAVE_POLKIT" = "xyes"; then
   suid=no
fi

AC_ARG_WITH([cpufreq-lib],
	AS_HELP_STRING([--with-cpufreq-lib=lib], [library to use for cpufreq applet @<:@default=cpufreq@:>@]),
	[with_cpufreq_lib=$withval], [with_cpufreq_lib="cpufreq"])

AC_CHECK_HEADER(cpufreq.h, have_libcpufreq=yes, have_libcpufreq=no)
LIBCPUFREQ_LIBS=
if test "x$have_libcpufreq" = "xyes"; then
   AC_DEFINE([HAVE_LIBCPUFREQ], [1], [Have libcpufreq.])
   LIBCPUFREQ_LIBS="-l$with_cpufreq_lib"
fi
AM_CONDITIONAL(HAVE_LIBCPUFREQ, test x$have_libcpufreq = xyes)
AC_SUBST(LIBCPUFREQ_LIBS)

build_cpufreq_applet=no

if test x$disable_cpufreq = xno; then
   case "${host}" in
      *linux*)
         build_cpufreq_applet=yes
	 ;;
      *)
         AC_MSG_WARN([${host} is not supported by cpufreq applet, not building])
	 build_cpufreq_applet=no
	 ;;
   esac
fi

AM_CONDITIONAL(BUILD_CPUFREQ_APPLET, test x$build_cpufreq_applet = xyes)
AM_CONDITIONAL(BUILD_CPUFREQ_SELECTOR, test x$enable_selector = xyes)
AM_CONDITIONAL(CPUFREQ_SELECTOR_SUID, test x$suid = xyes)

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

dnl ***************************************************************************
dnl *** timer applet specific checks                                        ***
dnl ***************************************************************************

AM_CONDITIONAL(BUILD_TIMER, test "x$HAVE_LIBNOTIFY" = "xyes")

dnl ***************************************************************************
dnl *** Check if IPv6 is available                                          ***
dnl ***************************************************************************
AC_MSG_CHECKING([whether to enable ipv6])
AC_ARG_ENABLE(ipv6, [  --enable-ipv6 enable IPv6 extensions], ,enable_ipv6=yes)
if test $enable_ipv6 = yes; then

  dnl Code for checking presence of AF_INET6 on the system.
  AC_TRY_COMPILE([
    #include <sys/types.h>
    #include <sys/socket.h>], [
    socket(AF_INET6, SOCK_STREAM, 0)
    ],
    have_ipv6=yes,
    have_ipv6=no
  )
  AC_MSG_RESULT($have_ipv6)
  if test $have_ipv6 = yes; then
      AC_DEFINE(ENABLE_IPV6, 1, [Define if system is IPv6 enabled])
  fi
fi

dnl ***************************************************************************
dnl *** Check for getaddrinfo                                               ***
dnl ***************************************************************************
have_getaddrinfo=no
AC_CHECK_FUNC(getaddrinfo, have_getaddrinfo=yes)
if test $have_getaddrinfo != yes; then
  # getaddrinfo is not in the default libraries.  See if it's in some other.
  for lib in bsd socket inet; do
    AC_CHECK_LIB($lib, getaddrinfo, [LIBS="$LIBS -l$lib";have_getaddrinfo=yes; break])
  done
fi
if test $have_getaddrinfo = yes; then
  AC_DEFINE(HAVE_GETADDRINFO, 1, [Define if getaddrinfo() exists on the system])
fi

dnl ***************************************************************************
dnl *** Check for presence of tm.tm_gmtoff on the system                    ***
dnl ***************************************************************************
AC_CHECK_MEMBER([struct tm.tm_gmtoff],[struct_tm_tm_gmtoff=true],[struct_tm_tm_gmtoff=false],[#include <time.h>])
case $struct_tm_tm_gmtoff in
        true)
                AC_DEFINE_UNQUOTED(HAVE_TM_TM_GMOFF, 1, Have the tm.tm_gmtoff member.)
                ;;
        *)
                AC_CHECK_DECL(timezone,AC_DEFINE(HAVE_TIMEZONE,1,Have timezone),,
[
#include <time.h>
])
                ;;
esac

dnl ***************************************************************************
dnl *** _NL_MEASUREMENT_MEASUREMENT is an enum and not a define             ***
dnl ***************************************************************************
AC_MSG_CHECKING([for _NL_MEASUREMENT_MEASUREMENT])
AC_TRY_LINK([#include <langinfo.h>], [
char c;
c = *((unsigned char *)  nl_langinfo(_NL_MEASUREMENT_MEASUREMENT));
], gtk_ok=yes, gtk_ok=no)
AC_MSG_RESULT($gtk_ok)
if test "$gtk_ok" = "yes"; then
  AC_DEFINE([HAVE__NL_MEASUREMENT_MEASUREMENT], [1],
            [Define if _NL_MEASUREMENT_MEASUREMENT is available])
fi

dnl **************************************************************************
dnl Process .in files
dnl **************************************************************************

AC_CONFIG_FILES([
  Makefile

  po/Makefile.in

  accessx-status/help/Makefile
  accessx-status/Makefile
  accessx-status/pixmaps/Makefile
  accessx-status/src/Makefile

  battstat/Makefile
  battstat/help/Makefile
  battstat/sounds/Makefile
  battstat/src/Makefile
  battstat/src/apmlib/Makefile

  brightness/Makefile
  brightness/icons/Makefile
  brightness/icons/16x16/Makefile
  brightness/icons/16x16/apps/Makefile
  brightness/icons/16x16/status/Makefile
  brightness/icons/22x22/Makefile
  brightness/icons/22x22/apps/Makefile
  brightness/icons/22x22/status/Makefile
  brightness/icons/24x24/Makefile
  brightness/icons/24x24/apps/Makefile
  brightness/icons/24x24/status/Makefile
  brightness/icons/32x32/Makefile
  brightness/icons/32x32/apps/Makefile
  brightness/icons/32x32/status/Makefile
  brightness/icons/48x48/Makefile
  brightness/icons/48x48/apps/Makefile
  brightness/icons/48x48/status/Makefile
  brightness/icons/scalable/Makefile
  brightness/icons/scalable/apps/Makefile
  brightness/icons/scalable/status/Makefile
  brightness/src/Makefile

  charpick/Makefile
  charpick/help/Makefile
  charpick/src/Makefile

  command/Makefile
  command/src/Makefile

  cpufreq/help/Makefile
  cpufreq/Makefile
  cpufreq/pixmaps/Makefile
  cpufreq/src/cpufreq-selector/Makefile
  cpufreq/src/Makefile

  drivemount/Makefile
  drivemount/help/Makefile
  drivemount/src/Makefile

  inhibit/Makefile
  inhibit/icons/Makefile
  inhibit/icons/16x16/Makefile
  inhibit/icons/16x16/apps/Makefile
  inhibit/icons/16x16/status/Makefile
  inhibit/icons/22x22/Makefile
  inhibit/icons/22x22/apps/Makefile
  inhibit/icons/22x22/status/Makefile
  inhibit/icons/24x24/Makefile
  inhibit/icons/24x24/apps/Makefile
  inhibit/icons/24x24/status/Makefile
  inhibit/icons/32x32/Makefile
  inhibit/icons/32x32/apps/Makefile
  inhibit/icons/32x32/status/Makefile
  inhibit/icons/48x48/Makefile
  inhibit/icons/48x48/apps/Makefile
  inhibit/icons/48x48/status/Makefile
  inhibit/icons/scalable/Makefile
  inhibit/icons/scalable/apps/Makefile
  inhibit/icons/scalable/status/Makefile
  inhibit/src/Makefile

  geyes/Makefile
  geyes/help/Makefile
  geyes/src/Makefile
  geyes/themes/Makefile

  gweather/Makefile
  gweather/help/Makefile
  gweather/src/Makefile

  invest-applet/data/art/Makefile
  invest-applet/data/Makefile
  invest-applet/help/Makefile
  invest-applet/invest/Makefile
  invest-applet/Makefile

  mini-commander/help/Makefile
  mini-commander/Makefile
  mini-commander/src/Makefile

  modem-lights/Makefile
  modem-lights/src/Makefile

  multiload/Makefile
  multiload/help/Makefile
  multiload/src/Makefile

  netspeed/data/Makefile
  netspeed/help/Makefile
  netspeed/icons/Makefile
  netspeed/icons/16x16/Makefile
  netspeed/icons/16x16/apps/Makefile
  netspeed/icons/16x16/devices/Makefile
  netspeed/icons/22x22/Makefile
  netspeed/icons/22x22/apps/Makefile
  netspeed/icons/24x24/Makefile
  netspeed/icons/24x24/apps/Makefile
  netspeed/icons/24x24/status/Makefile
  netspeed/icons/32x32/Makefile
  netspeed/icons/32x32/apps/Makefile
  netspeed/icons/48x48/Makefile
  netspeed/icons/48x48/apps/Makefile
  netspeed/icons/scalable/Makefile
  netspeed/icons/scalable/apps/Makefile
  netspeed/src/Makefile
  netspeed/Makefile

  sticky-notes/Makefile
  sticky-notes/data/Makefile
  sticky-notes/src/Makefile
  sticky-notes/help/Makefile
  sticky-notes/pixmaps/Makefile

  timer/Makefile
  timer/data/Makefile
  timer/src/Makefile

  tracker-search-bar/Makefile
  tracker-search-bar/src/Makefile
  tracker-search-bar/data/Makefile

  trash/help/Makefile
  trash/Makefile
  trash/src/Makefile
  trash/data/Makefile

  windowpicker/Makefile
  windowpicker/data/Makefile
  windowpicker/src/Makefile
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
echo "  Applets"
echo "    accessx-status ................: $HAVE_XKB"
echo "    batstat .......................: $build_battstat_applet"
echo "    brightness ....................: always"
echo "    charpick ......................: always"
echo "    command .......................: always"
echo "    cpufreq .......................: $build_cpufreq_applet"
echo "    drivemount ....................: always"
echo "    geyes .........................: always"
echo "    gweather ......................: $build_libgweather_applets"
echo "    inhibit .......................: always"
echo "    invest-applet .................: always"
echo "    mini-commander ................: $enable_mini_commander"
echo "    modem-lights ..................: $BUILD_MODEM_LIGHTS"
echo "    multiload .....................: $build_gtop_applets"
echo "    netspeed ......................: $build_gtop_applets"
echo "    sticky-notes ..................: $enable_stickynotes"
echo "    timer .........................: $HAVE_LIBNOTIFY"
echo "    tracker-search-bar ............: $have_tracker_search_bar"
echo "    trash .........................: always"
echo "    windowpicker ..................: always"
echo ""
echo "  charpick"
echo "    gucharmap support .............: $have_gucharmap"
echo ""
echo "  cpufreq"
echo "    building selector .............: $enable_selector"
echo "    using PolicyKit ...............: $HAVE_POLKIT"
echo "    enabling suid bit .............: $suid"
echo ""
echo "  Using DBUS ......................: $HAVE_DBUS"
echo "  Using UPOWER ....................: $HAVE_UPOWER"
echo "  Enabling IPv6 ...................: $have_ipv6"
echo ""
