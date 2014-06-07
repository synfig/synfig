## Copyright (c) 2004-2007 Daniel Elstner <daniel.kitta@gmail.com>
##
## This file is part of danielk's Autostuff.
##
## danielk's Autostuff is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2 of the License, or (at
## your option) any later version.
##
## danielk's Autostuff is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
## or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with danielk's Autostuff; if not, write to the Free Software Foundation,
## Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#serial 20070116

## DK_ARG_ENABLE_WARNINGS(variable, min-flags, max-flags, [deprecation-prefixes])
##
## Provide the --enable-warnings configure argument, set to "min" by default.
## <min-flags> and <max-flags> should be space-separated lists of compiler
## warning flags to use with --enable-warnings=min or --enable-warnings=max,
## respectively. Warning level "fatal" is the same as "max" but in addition
## enables -Werror mode.
##
## If not empty, <deprecation-prefixes> should be a list of module prefixes
## which is expanded to -D<module>_DISABLE_DEPRECATED flags if fatal warnings
## are enabled, too.
##
AC_DEFUN([DK_ARG_ENABLE_WARNINGS],
[dnl
m4_if([$3],, [AC_FATAL([3 arguments expected])])[]dnl
dnl
AC_ARG_ENABLE([warnings], [AS_HELP_STRING(
  [--enable-warnings=@<:@min|max|fatal|no@:>@],
  [control compiler pickyness @<:@min@:>@])],
  [dk_enable_warnings=$enableval],
  [dk_enable_warnings=min])[]dnl

dk_lang=
case $ac_compile in
  *'$CXXFLAGS '*)
    dk_lang='C++'
    dk_cc=$CXX
    dk_conftest=conftest.${ac_ext-cc}
    ;;
  *'$CFLAGS '*)
    dk_lang=C
    dk_cc=$CC
    dk_conftest=conftest.${ac_ext-c}
    ;;
esac

AS_IF([test "x$dk_lang" != x],
[
  AC_MSG_CHECKING([which $dk_lang compiler warning flags to use])

  case $dk_enable_warnings in
    no) dk_warning_flags=;;
    max) dk_warning_flags="$3";;
    fatal) dk_warning_flags="$3 -Werror";;
    *) dk_warning_flags="$2";;
  esac

  dk_deprecation_flags=
m4_if([$4],,, [
  AS_IF([test "x$dk_enable_warnings" = xfatal],
  [
    dk_deprecation_prefixes="$4"
    for dk_prefix in $dk_deprecation_prefixes
    do
      dk_deprecation_flags="${dk_deprecation_flags}-D${dk_prefix}_DISABLE_DEPRECATED "
    done
  ])
])[]dnl
  dk_tested_flags=

  AS_IF([test "x$dk_warning_flags" != x],
  [
    # Keep in mind that the dummy source must be devoid of any
    # problems that might cause diagnostics.
    AC_LANG_CONFTEST([AC_LANG_SOURCE(
      [[int main(int argc, char** argv) { return (argv != 0) ? argc : 0; }]])])

    for dk_flag in $dk_warning_flags
    do
      # Test whether the compiler accepts the flag. GCC doesn't bail
      # out when given an unsupported flag but prints a warning, so
      # check the compiler output instead.
      dk_cc_out=`$dk_cc $dk_tested_flags $dk_flag -c "$dk_conftest" 2>&1 || echo failed`
      rm -f "conftest.${OBJEXT-o}"

      AS_IF([test "x$dk_cc_out" = x],
      [
        AS_IF([test "x$dk_tested_flags" = x],
              [dk_tested_flags=$dk_flag],
              [dk_tested_flags="$dk_tested_flags $dk_flag"])
      ], [
        echo "$dk_cc_out" >&AS_MESSAGE_LOG_FD
      ])
    done

    rm -f "$dk_conftest"
  ])
  dk_all_flags=$dk_deprecation_flags$dk_tested_flags
  AC_SUBST([$1], [$dk_all_flags])

  test "x$dk_all_flags" != x || dk_all_flags=none
  AC_MSG_RESULT([$dk_all_flags])
])
])
