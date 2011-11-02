AC_PREREQ(2.53)
AC_INIT(window-picker-applet, 0.6.11, [])
AM_INIT_AUTOMAKE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR(src/applet.c)
AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

dnl ==========================================================================
dnl i18n
dnl ==========================================================================

GETTEXT_PACKAGE=window-picker-applet
AC_SUBST(GETTEXT_PACKAGE)
AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/po/LINGUAS'])
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define to the Gettext package name])
ALL_LINGUAS="`cat "$srcdir/po/LINGUAS" | grep -v '^#'`"

IT_PROG_INTLTOOL([0.34])

AM_GLIB_GNU_GETTEXT

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall -Werror"
fi

AC_SUBST(GCC_FLAGS)


PKG_CHECK_MODULES(DEPS, 
                  libpanelapplet-2.0
                  gtk+-2.0
                  gconf-2.0 
                  libwnck-1.0
                  )
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AC_PATH_PROG(GCONFTOOL, gconftool-2)
AM_GCONF_SOURCE_2

AC_OUTPUT([
Makefile
data/Makefile
src/Makefile
po/Makefile.in
])
