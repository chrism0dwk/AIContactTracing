AC_INIT(configure.ac)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(aicontacttracing, 1.0)

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AM_PROG_LIBTOOL

dnl Xerces-c locations

XERCES_LIBS="-lxerces-c"
AC_ARG_WITH([xercesc-dir], 
[AS_HELP_STRING([--with-xercesc-dir=DIR],[Path to Xerces-c DIR/lib and DIR/include. Defaults to system installation])],
[
	if test "$withval" != "yes" -a "$withval" != ""; then
		XERCES_LIBS="-L$withval/lib -lxerces-c"
		XERCES_CXXFLAGS="-I$withval/include"
		XERCES_CPPFLAGS="-I$withval/include"
	fi
])


dnl Check for GSL presence
AC_SEARCH_LIBS([gsl_log1p],[gsl],,echo "LDFLAGS: $LDFLAGS"; $srcdir/missing GSL; exit,-lgslcblas)
GSL_LIBS="-lgsl -lgslcblas"


dnl Check for Boost presence
AC_CHECK_HEADER([boost/property_tree/ptree.hpp],,echo "CXXFLAGS: $CXXFLAGS"; $srcdir/missing Boost; exit, )
AX_BOOST_PROGRAM_OPTIONS


dnl wxWidgets configuration

WXCONFIG=wx-config
AC_ARG_WITH(wx-config,
[AS_HELP_STRING([--with-wx-config=FILE],[Use the given path to wx-config when determining
                            wxWidgets configuration; defaults to "wx-config"])],
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WXCONFIG=$withval
    fi
])

wxversion=0

AC_DEFUN([WXTEST],
[
	AC_REQUIRE([AC_PROG_AWK])
	AC_MSG_CHECKING([wxWidgets version])
	if wxversion=`$WXCONFIG --version`; then
		AC_MSG_RESULT([$wxversion])
	else
		AC_MSG_RESULT([not found])
		AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
	fi])

# Call WXTEST func
dnl WXTEST

# Verify minimus requires
dnl vers=`echo $wxversion | $AWK 'BEGIN { FS = "."; } { printf "% d", ($1 * 1000 + $2) * 1000 + $3;}'`
dnl if test -n "$vers" && test "$vers" -ge 2003003; then
dnl 	WX_CPPFLAGS="`$WXCONFIG --cppflags`"
dnl 	WX_CXXFLAGS="`$WXCONFIG --cxxflags | sed -e 's/-fno-exceptions//'`"
dnl 	WX_LIBS="`$WXCONFIG --libs`"
dnl else
dnl 	AC_MSG_ERROR([wxWidgets 2.3.3 or newer is required])
dnl fi


CPPFLAGS="-fopenmp -fomit-frame-pointer -Wall $CPPFLAGS $WX_CPPFLAGS $XERCES_CPPFLAGS -g"
CXXFLAGS="-fopenmp -fomit-frame-pointer -Wall $CXXFLAGS $WX_CPPFLAGS $XERCES_CPPFLAGS -g"
LIBS="$XERCES_LIBS $GSL_LIBS $BOOST_LIBS"




dnl *******CHECK FUNCTIONALITY OF LIBRARIES*************

dnl Xerces-c 3.0 or greater
AC_MSG_CHECKING([for Xerces-c version > 3.0])
AC_LINK_IFELSE([
                 AC_LANG_PROGRAM([#include <xercesc/dom/DOM.hpp>],
                                [XERCES_CPP_NAMESPACE_USE; DOMLSSerializer* serializer;])
                ],
                [AC_MSG_RESULT([found])],
                [AC_MSG_FAILURE(Required Xerces-c > 3.0 not found: $LIBS)])
            


AC_SUBST(WX_LIBS)


AC_OUTPUT(Makefile src/Makefile src/common/Makefile src/data/Makefile \
	src/data/config/Makefile src/mcmc/Makefile src/test/Makefile \
	src/utils/I1_Freq/Makefile src/utils/Makefile src/utils/Python/Makefile        src/utils/R2_calc/Makefile \
        src/utils/contactRate/Makefile src/utils/contactSim/Makefile \
	src/utils/contactTest/Makefile src/utils/occultFreq/Makefile \
	src/sim/Makefile src/sim/gillespie/Makefile)
