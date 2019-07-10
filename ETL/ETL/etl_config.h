#ifndef __ETL_CONFIG_H
#define __ETL_CONFIG_H

#include <ETL/etl_profile.h>
#include <utility>

#define ETL_DIRECTORY_SEPARATORS	"/\\"
#define ETL_DIRECTORY_SEPARATOR0	'/'
#define ETL_DIRECTORY_SEPARATOR1	'\\'

#define ETL_DIRECTORY_SEPARATOR		ETL_DIRECTORY_SEPARATOR0

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
