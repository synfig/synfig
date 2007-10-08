dnl AC_LIBXMLPP([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])

AC_DEFUN([AM_LIBXMLPP],
[

AC_PATH_PROG(XMLPP_CONFIG,xml++-config,no)

AC_MSG_CHECKING(for libxml++)

if $XMLPP_CONFIG --libs print > /dev/null 2>&1; then
    AC_MSG_RESULT(yes)
    LIBXMLPP_CFLAGS=`xml++-config --cflags`
    LIBXMLPP_LIBS=`xml++-config --libs`
    AC_SUBST(LIBXMLPP_CFLAGS)
    AC_SUBST(LIBXMLPP_LIBS)
    ifelse([$1], , :, [$1])
else
    AC_MSG_RESULT(no)
    ifelse([$2], , , [$2])
fi

])

