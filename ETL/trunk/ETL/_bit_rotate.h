/*! ========================================================================
** Extended Template Library
** Bit Rotation Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** These template functions have not yet been throughly tested,
** and may be inaccurate or just plain wrong. You have been warned.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__BIT_ROTATE_H
#define __ETL__BIT_ROTATE_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

template <typename T> T
rot_left(const T &val, const int &bits=1)
{
	return (T)( ((unsigned)val<<bits)+((unsigned)val>>(sizeof(T)*8-bits)) );
}

template <typename T> T
rot_right(const T &val, const int &bits=1)
{
	return (T)( ((unsigned)val>>bits)+((unsigned)val<<(sizeof(T)*8-bits)) );
}

_ETL_END_NAMESPACE

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif

