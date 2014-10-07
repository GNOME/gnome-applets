# This is the configuration file for gnu autoconf
# see www.gnu.org/software/autoconf/manual/autoconf.html
# for an explanation of the macros that are used here

AC_PREREQ([2.68])
AC_INIT([window-picker-applet],[0.6.99],[sbastig@gmx.net],[window-picker-applet],[https://github.com/lanoxx/window-picker-applet])
AM_INIT_AUTOMAKE([foreign 1.11.1])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR(src/applet.c)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
#AM_MAINTAINER_MODE

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AC_HEADER_STDC
LT_PREREQ([2.2.6])
LT_INIT

dnl ==========================================================================
dnl i18n
dnl ==========================================================================

# Define GETTEXT_PACKAGE=window-picker-applet and also replace all
# occurences of @GETTEXT_PACKAGE@ with window-picker-applet
AC_SUBST([GETTEXT_PACKAGE], [window-picker-applet])
AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/po/LINGUAS'])
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define to the Gettext package name])
ALL_LINGUAS="`cat "$srcdir/po/LINGUAS" | grep -v '^#'`"

IT_PROG_INTLTOOL([0.34])

AM_GLIB_GNU_GETTEXT

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall -Werror"
fi

APPLETDIR=`pkg-config --variable=libpanel_applet_dir libpanelapplet-4.0`

GLIB_GSETTINGS

AC_SUBST(GCC_FLAGS)

GTK_REQUIRED=3.0.0
PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= $GTK_REQUIRED])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(DESKTOP_GSETTINGS, gsettings-desktop-schemas)
AC_SUBST(DESKTOP_GSETTINGS)

PKG_CHECK_MODULES(DEPS, 
                  libpanelapplet-4.0
                  gtk+-3.0
                  libwnck-3.0
                  )
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

#AC_PATH_PROG(GCONFTOOL, gconftool-2)
#AM_GCONF_SOURCE_2

AC_CONFIG_FILES([
Makefile
data/Makefile
src/Makefile
po/Makefile.in
])
AC_OUTPUT
