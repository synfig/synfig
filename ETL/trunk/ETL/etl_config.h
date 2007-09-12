

#ifndef __ETL_CONFIG_H
#define __ETL_CONFIG_H

#include "etl_profile.h"
#include <utility>

#ifndef ETL_NAMESPACE
# define ETL_NAMESPACE 			etl
#endif

#if defined(WORDS_BIGENDIAN) && !defined(ETL_BIGENDIAN)
#define ETL_BIGENDIAN
#endif

#ifdef	WIN32
#define ETL_DIRECTORY_SEPARATOR		'\\'
#else
#define ETL_DIRECTORY_SEPARATOR		'/'
#endif

#ifndef ETL_FLAG_NONAMESPACE
# define _ETL					ETL_NAMESPACE
# define _ETL_BEGIN_NAMESPACE	namespace _ETL {
# define _ETL_END_NAMESPACE		};
# define _STD_BEGIN_NAMESPACE	namespace std {
# define _STD_END_NAMESPACE		};
#else
# define _ETL
# define _ETL_BEGIN_NAMESPACE
# define _ETL_END_NAMESPACE
# define _STD_BEGIN_NAMESPACE
# define _STD_END_NAMESPACE
#endif

#define _ETL_BEGIN_CDECLS		extern "C" {
#define _ETL_END_CDECLS			}

#ifdef _REENTRANT
#define ETL_REENTRANT	1
#endif

/* If __FUNC__ is not defined,
** try to define it. If we cannot,
** then just leave it undefined.
*/
#ifndef __FUNC__
/*
 * # if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define __FUNC__	__PRETTY_FUNCTION__
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __FUNC__	__func__
#  endif
# endif
*/
#endif

#ifdef __GNUG__
#define ETL_DEPRECATED_FUNCTION		__attribute__ ((deprecated))
#else
#define ETL_DEPRECATED_FUNCTION
#endif

#ifndef NULL
#define NULL	0
#endif

#endif
