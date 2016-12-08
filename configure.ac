AC_PREREQ([2.63])

AC_INIT([atk-cocoa], [0.1])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([atk-cocoa/acelement.h])

AM_INIT_AUTOMAKE

# Support silent build rules, requires at least automake-1.11. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_OBJC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AM_PATH_GLIB_2_0(2.31.2, :, AC_MSG_ERROR([
*** GLIB 2.31.2 or better required]), gobject)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.18.0,,
		  AC_MSG_ERROR([GTK+-2.18 is required]))

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

GTK_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
AC_SUBST(GTK_VERSION)

AC_CONFIG_FILES([
atk-cocoa/Makefile
])

AC_OUTPUT
