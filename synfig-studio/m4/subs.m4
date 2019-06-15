
## AC_ARG_WARNINGS()
##
## Provide the --enable-warnings configure argument, set to 'minimum'
## by default.
##
AC_DEFUN([AC_ARG_WARNINGS],
[
  AC_ARG_ENABLE([warnings],
      [  --enable-warnings=[[none|minimum|maximum|hardcore]]
                          Control compiler pickyness.  [[default=maximum]]],
      [gtkmm_enable_warnings="$enableval"],
      gtkmm_enable_warnings="maximum")

  AC_MSG_CHECKING([for compiler warning flags to use])

  gtkmm_warning_flags=''

  # -W is now known as -Wextra, but that's not known by gcc 2 or 3
  case "$gtkmm_enable_warnings" in
    none|no)     gtkmm_warning_flags='';;
    minimum|yes) gtkmm_warning_flags='-Wall -Wno-unused-parameter';;
    maximum)     gtkmm_warning_flags='-W -Wall';;
    hardcore)    gtkmm_warning_flags='-W -Wall -Werror';;
  esac

  gtkmm_use_flags=''

  if test "x$gtkmm_warning_flags" != "x"
  then
    echo 'int foo() { return 0; }' > conftest.cc

    for flag in $gtkmm_warning_flags
    do
      # Test whether the compiler accepts the flag.  GCC doesn't bail
      # out when given an unsupported flag but prints a warning, so
      # check the compiler output instead.
      gtkmm_cxx_out="`$CXX $flag -c conftest.cc 2>&1`"
      rm -f conftest.$OBJEXT
      test "x${gtkmm_cxx_out}" = "x" && \
        gtkmm_use_flags="${gtkmm_use_flags:+$gtkmm_use_flags }$flag"
    done

    rm -f conftest.cc
    gtkmm_cxx_out=''
  fi

  if test "x$gtkmm_use_flags" != "x"
  then
    for flag in $gtkmm_use_flags
    do
      case " $CXXFLAGS " in
        *" $flag "*) ;; # don't add flags twice
        *)           CXXFLAGS="${CXXFLAGS:+$CXXFLAGS }$flag";;
      esac
    done
  else
    gtkmm_use_flags='none'
  fi

  AC_MSG_RESULT([$gtkmm_use_flags])
])




AC_DEFUN([AC_ARG_DEBUG],
[
	AC_MSG_CHECKING([for debug flags])

	AC_ARG_ENABLE(debug,[  --enable-debug           Build in debugging mode],[
		debug=$enableval
	],[
		debug="no"
	])
	debug_flags=''

	case "$debug" in
		yes)
			debug_flags="-D_DEBUG -g -O0"
			CXXFLAGS="`echo $CXXFLAGS | sed s:-O.::` $debug_flags -fno-inline"
			CFLAGS="`echo $CFLAGS | sed s:-O.::` $debug_flags"
		;;
		no|*)
			debug_flags="-DNDEBUG"
			CXXFLAGS="`echo $CXXFLAGS | sed 's:-g[[a-z-]]*\s::g' | sed 's:-g[[a-z-]]*$::'` $debug_flags"
			CFLAGS="`echo $CFLAGS | sed 's:-g[[a-z-]]*\s::g' | sed 's:-g[[a-z-]]*$::'` $debug_flags"
		;;
	esac

	AC_MSG_RESULT([$debug_flags])
])




AC_DEFUN([AC_ARG_OPTIMIZATION],
[
	AC_MSG_CHECKING([for optimization flags])

	AC_ARG_ENABLE(optimization,[  --enable-optimization=[[0,1,2,3,4]] Select optimization level (default=2)],[
		optimization=$enableval
	],[
		optimization="2"
	])
	optimization_flags=''
	case "$optimization" in
		0|no)	optimization_flags="-O0";;
		1) 		optimization_flags="-O1";;
		2|yes)	optimization_flags="-O2";;
		pass1)	optimization_flags="-O2 -fprofile-arcs";;
		pass2)	optimization_flags="-O2 -fbranch-probabilities";;
		3) 		optimization_flags="-O3";;
		*) 		optimization_flags="-O4";;
	esac
	CXXFLAGS="`echo $CXXFLAGS | sed 's:-O.::g'` $optimization_flags"
	CFLAGS="`echo $CFLAGS | sed 's:-O.::g'` $optimization_flags"
	AC_MSG_RESULT([$optimization_flags])	
])

AC_DEFUN([AC_ARG_PROFILE_ARCS],
[
	AC_MSG_CHECKING([for arc profiling])

	AC_ARG_ENABLE(profile-arcs,[  --enable-profile-arcs      Enable arc profiling],[
		profile_arcs=$enableval
	],[
		profile_arcs=no
	])
	
	if test $profile_arcs = "yes" ; then {
		CXXFLAGS="$CXXFLAGS -fprofile-arcs";
		CFLAGS="$CFLAGS -fprofile-arcs";
	} ; fi
		
	AC_MSG_RESULT([$profile_arcs])	
])

AC_DEFUN([AC_ARG_BRANCH_PROBABILITIES],
[
	AC_MSG_CHECKING([for branch-probabilities])

	AC_ARG_ENABLE(branch-probabilities,[  --enable-branch-probabilities      Enable branch-probabilities],[
		branch_probabilities=$enableval
	],[
		branch_probabilities=no
	])
	
	if test $branch_probabilities = "yes" ; then {
		CXXFLAGS="$CXXFLAGS -fbranch-probabilities";
		CFLAGS="$CFLAGS -fbranch-probabilities";
	} ; fi
		
	AC_MSG_RESULT([$branch_probabilities])	
])

AC_DEFUN([AC_ARG_PROFILING],
[
	AC_MSG_CHECKING([for profiling])

	AC_ARG_ENABLE(profiling,[  --enable-profiling      Enable profiling using gprof],[
		profiling=$enableval
	],[
		profiling=no
	])
	
	if test $profiling = "yes" ; then {
		CFLAGS="$CFLAGS -pg";
		CXXFLAGS="$CXXFLAGS -pg";
		LDFLAGS="$LDFLAGS -pg";
		LIBS="$LIBS";
	} ; fi
		
	AC_MSG_RESULT([$profiling])	
])

MINGW_FLAGS="-mno-cygwin"


AC_DEFUN([AC_WIN32_QUIRKS],
[

case "$host" in
  *mingw*)
    AC_MSG_CHECKING([the flavor of the compiler])
    if ( $CC --version | grep -q mingw ) ; then {
        AC_MSG_RESULT([compiler is mingw special])
        LIBTOOL_PATCH_SED="
            s/dir=\"\$absdir\"/dir=\`cygpath -d -m \"\$absdir\"\`/;
            s/absdir=\`cd \"\$dir\" && pwd\`/absdir=\`cygpath -d -m \"\$dir\"\`/;
            s/# We need an absolute path/dir=\`cygpath -d -m \"\$dir\"\` # We need an absolute path/;
            s- /usr/lib- C:/mingw/lib-g;
            s-\"/lib -\"C:/mingw/lib -g;
            s- /lib/ - -g;
        ";
        sys_lib_dlsearch_path_spec="C:/mingw/lib"
        ac_default_prefix=`cygpath -d -m "$ac_default_prefix"`;
    } else {
    AC_MSG_RESULT([compiler is cygwin stock, adding -mno-cygwin])
    CPP="$CPP $MINGW_FLAGS"
    CC="$CC $MINGW_FLAGS"
    CXX="$CXX $MINGW_FLAGS -L/usr/$host/lib -I/usr/include/c++/3.3.3/$host"
    CXXCPP="$CXXCPP $MINGW_FLAGS"


} ; fi

    LTCC="gcc"
    CXXFLAGS="$CXXFLAGS -Wno-cpp -LC:/GTK/lib"
    CFLAGS="$CFLAGS -LC:/GTK/lib"
    LDFLAGS="$LDFLAGS -lole32 -Wl,-no-undefined -Wl,--export-all-symbols -Wl,--subsystem=windows -Wl,--enable-runtime-pseudo-reloc"
dnl    LDFLAGS="$LDFLAGS -lole32 -Wl,-no-undefined -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--subsystem=console -Wl,--enable-runtime-pseudo-reloc"
    ;;
  *cygwin*)
    LDFLAGS="$LDFLAGS -lole32 -Wl,-no-undefined -Wl,--export-all-symbols"
dnl    LDFLAGS="$LDFLAGS -lole32 -Wl,-no-undefined -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--subsystem=console"
    CXXFLAGS="$CXXFLAGS -I/target/include"
    CFLAGS="$CFLAGS -I/target/include"
    ;;
  powerpc-apple*)
    echo Adding mac-specific optimization flags. . .
    CXXFLAGS="$CXXFLAGS $G5OPTFLAGS"
    ;;
esac


])
