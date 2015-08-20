dnl ***************************************************************************
dnl *** configure.in for GNOME-APPLETS                                      ***
dnl ***************************************************************************
AC_INIT(gnome-applets, 3.17.2)
AC_PREREQ(2.59)

AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz tar-ustar])
AC_CONFIG_MACRO_DIR(m4)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl ***************************************************************************
dnl *** Minimum library versions for GNOME-APPLETS                          ***
dnl ***************************************************************************
GTK_REQUIRED=3.15.2
GLIB_REQUIRED=2.44.0
GIO_REQUIRED=2.26.0
LIBPANEL_REQUIRED=3.16.1
LIBGTOP_REQUIRED=2.11.92
LIBXKLAVIER_REQUIRED=4.0
LIBWNCK_REQUIRED=2.91.0
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

AM_MAINTAINER_MODE

IT_PROG_INTLTOOL([0.35.0])
PKG_PROG_PKG_CONFIG([0.19])

AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources], [no])
if test x$GLIB_COMPILE_RESOURCES = xno; then
  AC_MSG_ERROR([Could not find a glib-compile-resources in your PATH])
fi

AC_PROG_CC
AC_ISC_POSIX
AC_STDC_HEADERS
AM_PROG_LIBTOOL
AC_PATH_XTRA
AM_PATH_PYTHON(3)

X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
AC_SUBST(X_LIBS)

dnl Check for libm
LT_LIB_M

GNOME_COMPILE_WARNINGS

dnl Check for non-portable headers
AC_CHECK_HEADERS([pty.h values.h])

dnl ***************************************************************************
dnl *** --enable-more-warnings turns on more compiler warnings              ***
dnl ***************************************************************************
AC_ARG_ENABLE(more-warnings,
[  --enable-more-warnings  Maximum compiler warnings],
set_more_warnings="$enableval",[
    set_more_warnings=no
])
AC_MSG_CHECKING(for more warnings, including -Werror)
if test "$GCC" = "yes" -a "$set_more_warnings" != "no"; then
    AC_MSG_RESULT(yes)
    CFLAGS="\
    -Wall \
    -Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes \
    -Wnested-externs -Wpointer-arith \
    -Wcast-align -Wsign-compare \
    -std=gnu89 \
    $CFLAGS"

    for option in -Wno-strict-aliasing -Wno-sign-compare; do
        SAVE_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $option"
        AC_MSG_CHECKING([whether gcc understands $option])
        AC_TRY_COMPILE([], [],
            has_option=yes,
            has_option=no,)
        if test $has_option = no; then
            CFLAGS="$SAVE_CFLAGS"
        fi
        AC_MSG_RESULT($has_option)
        unset has_option
        unset SAVE_CFLAGS
    done
    unset option
else
    AC_MSG_RESULT(no)
fi
dnl ***************************************************************************

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

LIBPANEL_APPLET_API_VERSION=`$PKG_CONFIG --variable=libpanel_applet_api_version libpanel-applet`
AC_SUBST(LIBPANEL_APPLET_API_VERSION)

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

	LT_LIB_M
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
AC_COMPILE_IFELSE([
 		#include <iwlib.h>
                 #ifndef IWEVGENIE
                 #error "not found"
                 #endif],
                ac_have_iwevgenie=yes
)
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

dnl -- check for Python/GNOME modules (optional) ------------------------------
PKG_CHECK_MODULES(PYGOBJECT, pygobject-3.0, HAVE_PYGOBJECT=yes, HAVE_PYGOBJECT=no)

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
AC_DEFINE_DIR(GNOMELOCALEDIR, "${datadir}/locale", [locale directory])
AM_GLIB_GNU_GETTEXT

# this is the directory where the *.{mo,gmo} files are installed
gnomelocaledir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(gnomelocaledir)

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
dnl *** invest-applet specific checks                                       ***
dnl ***************************************************************************

AM_CONDITIONAL(BUILD_INVEST_APPLET, test "x$HAVE_PYGOBJECT" = "xyes")

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

dnl ***************************************************************************
dnl *** Honour aclocal flags                                                ***
dnl ***************************************************************************
ACLOCAL_AMFLAGS="\${ACLOCAL_FLAGS}"
AC_SUBST(ACLOCAL_AMFLAGS)

dnl ***************************************************************************
dnl *** Set install directories                                             ***
dnl ***************************************************************************
AC_ARG_WITH([pythondir],
	AS_HELP_STRING([--with-pythondir=DIR], [installation path for private Python modules @<:@auto@:>@]),
	[ac_with_pythondir=$withval], [ac_with_pythondir=""])
if test "$ac_with_pythondir" != ""; then
	pythondir=${ac_with_pythondir}
fi

AC_MSG_NOTICE([installing private Python modules in $pythondir])
AC_SUBST(pythondir)

iconsdir="${datadir}/icons"
AC_SUBST(iconsdir)
pixmapsdir="${datadir}/pixmaps"
AC_SUBST(pixmapsdir)
pkgdatadir="${datadir}/gnome-applets"
AC_SUBST(pkgdatadir)
builderdir="${pkgdatadir}/builder"
AC_SUBST(builderdir)

AC_DEFINE_DIR(GNOME_ICONDIR, "${iconsdir}", [Gnome Icon Directory])
AC_DEFINE_DIR(GNOME_PIXMAPSDIR, "${pixmapsdir}", [Gnome Pixmaps Directory])
AC_DEFINE_DIR(GTK_BUILDERDIR, "${builderdir}", [GtkBuilder Resources Directory])
AC_DEFINE_DIR(DATADIR, "${datadir}", [datadir])
AC_DEFINE_DIR(PKG_DATA_DIR, "${pkgdatadir}", [pkgdatadir])
AC_DEFINE_DIR(SYSCONFDIR, "${sysconfdir}", [sysconfdir])
AC_DEFINE_DIR(LIBDIR, "${libdir}", [libdir])
AC_DEFINE_DIR(PREFIX, "$prefix", [install prefix])

AS_AC_EXPAND(DATADIR, "${datadir}")
AS_AC_EXPAND(PKGDATADIR, "${pkgdatadir}")
AS_AC_EXPAND(PYTHONDIR, $pythondir)
AS_AC_EXPAND(BUILDERDIR, $builderdir)

AC_SUBST(VERSION)
AC_SUBST(PACKAGE)
AC_SUBST(PKGDATADIR)
AC_SUBST(DATADIR)
AC_SUBST(LIBDIR)
AC_SUBST(PYTHONDIR)
AC_SUBST(BUILDERDIR)

dnl ***************************************************************************
dnl *** Output Makefiles et al                                              ***
dnl ***************************************************************************
AC_OUTPUT([
gnome-applets.spec
Makefile
po/Makefile.in
accessx-status/help/Makefile
accessx-status/Makefile
accessx-status/pixmaps/Makefile
battstat/apmlib/Makefile
battstat/help/Makefile
battstat/Makefile
battstat/sounds/Makefile
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
charpick/help/Makefile
charpick/Makefile
cpufreq/help/Makefile
cpufreq/Makefile
cpufreq/pixmaps/Makefile
cpufreq/src/cpufreq-selector/Makefile
cpufreq/src/Makefile
drivemount/help/Makefile
drivemount/Makefile
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
geyes/help/Makefile
geyes/Makefile
geyes/themes/Makefile
gweather/help/Makefile
gweather/Makefile
invest-applet/data/art/Makefile
invest-applet/data/Makefile
invest-applet/help/Makefile
invest-applet/invest/defs.py
invest-applet/invest/Makefile
invest-applet/Makefile
man/Makefile
mini-commander/help/Makefile
mini-commander/Makefile
mini-commander/src/Makefile
modemlights/Makefile
multiload/help/Makefile
multiload/Makefile
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
stickynotes/help/Makefile
stickynotes/Makefile
stickynotes/pixmaps/Makefile
tracker-search-bar/Makefile
trashapplet/help/Makefile
trashapplet/Makefile
trashapplet/src/Makefile
windowpicker/Makefile
windowpicker/data/Makefile
windowpicker/src/Makefile
])

dnl ***************************************************************************
dnl *** Display Summary                                                     ***
dnl ***************************************************************************
echo "
gnome-applets-$VERSION configure summary:

	Prefix:				${prefix}
	Source code location:		${srcdir}
	Compiler:			${CC}

	Building:
	 - accessx-status		$HAVE_XKB
	 - battstat			$build_battstat_applet
	 - charpick			always
	 	- gucharmap support	$have_gucharmap
	 - cpufreq			$build_cpufreq_applet
	 	- building selector	$enable_selector
		- using PolicyKit       $HAVE_POLKIT
		- enabling suid bit	$suid
	 - drivemount			always
	 - geyes			always
	 - gweather			$build_libgweather_applets
	 - inhibit			always
	 - invest-applet		$HAVE_PYGOBJECT
	 - mini-commander		$enable_mini_commander
	 - modemlights			$BUILD_MODEM_LIGHTS
	 - multiload			$build_gtop_applets
	 - netspeed                     $build_gtop_applets
	 - stickynotes			$enable_stickynotes
	 - trashapplet			always
	 - tracker-search-bar:          $have_tracker_search_bar
	 - windowpicker			always

	Using DBUS:			$HAVE_DBUS
	Using UPOWER:			$HAVE_UPOWER
	Enabling IPv6:			$have_ipv6
" >&2
