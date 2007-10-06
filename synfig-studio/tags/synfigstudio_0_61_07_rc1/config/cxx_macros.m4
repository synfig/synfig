AC_DEFUN([AC_CXX_FUNCTION_NONTYPE_PARAMETERS],
[AC_CACHE_CHECK(whether the compiler supports function templates with non-type parameters,
ac_cv_cxx_function_nontype_parameters,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
template<class T, int N> class A {};
template<class T, int N> int f(const A<T,N>& x) { return 0; }
],[A<double, 17> z; return f(z);],
 ac_cv_cxx_function_nontype_parameters=yes, ac_cv_cxx_function_nontype_parameters=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_function_nontype_parameters" = yes; then
  AC_DEFINE(HAVE_FUNCTION_NONTYPE_PARAMETERS,,
            [define if the compiler supports function templates with non-type parameters])
fi
])

AC_DEFUN([AC_CXX_NAMESPACES],
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ac_cv_cxx_namespaces,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([namespace Outer { namespace Inner { int i = 0; }}],
                [using namespace Outer::Inner; return i;],
 ac_cv_cxx_namespaces=yes, ac_cv_cxx_namespaces=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_namespaces" = yes; then
  AC_DEFINE(HAVE_NAMESPACES,,[define if the compiler implements namespaces])
fi
])

AC_DEFUN([AC_CXX_HAVE_COMPLEX],
[AC_CACHE_CHECK(whether the compiler has complex<T>,
ac_cv_cxx_have_complex,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <complex>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[complex<float> a; complex<double> b; return 0;],
 ac_cv_cxx_have_complex=yes, ac_cv_cxx_have_complex=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_complex" = yes; then
  AC_DEFINE(HAVE_COMPLEX,,[define if the compiler has complex<T>])
fi
])

AC_DEFUN([AC_CXX_HAVE_SSTREAM],
[AC_CACHE_CHECK(whether the compiler has stringstream,
ac_cv_cxx_have_sstream,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <sstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[stringstream message; message << "Hello"; return 0;],
 ac_cv_cxx_have_sstream=yes, ac_cv_cxx_have_sstream=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_sstream" = yes; then
  AC_DEFINE(HAVE_SSTREAM,,[define if the compiler has stringstream])
fi
])

AC_DEFUN([AC_CXX_MUTABLE],
[AC_CACHE_CHECK(whether the compiler supports the mutable keyword,
ac_cv_cxx_mutable,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
class A { mutable int i;
          public:
          int f (int n) const { i = n; return i; }
        };
],[A a; return a.f (1);],
 ac_cv_cxx_mutable=yes, ac_cv_cxx_mutable=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_mutable" = yes; then
  AC_DEFINE(HAVE_MUTABLE,,[define if the compiler supports the mutable keyword])
fi
])

