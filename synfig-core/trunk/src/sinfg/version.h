/* === S I N F G =========================================================== */
/*!	\file version.h
**	\brief Template Header
**
**	$Id: version.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_VERSION_H
#define __SINFG_VERSION_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/*! \def SINFG_VERSION
**	\brief Sinfg API Version
**
**	The macro SINFG_VERSION can be set to ensure
**	compile-time compatibility with future versions
**	of Sinfg. The first two digits are the major
**	version, the second two digits are the minor
**	version, and the last two digits are the
**	revision release.
*/
#ifndef SINFG_VERSION
#define SINFG_VERSION (006000)
#endif

/*!	Increment this value whenever
**	the library changes in a way
**	that breaks library compatibility
*/
#define SINFG_LIBRARY_VERSION	47

/*! \writeme */
#define SINFG_CHECK_VERSION()	sinfg::check_version_(SINFG_LIBRARY_VERSION,sizeof(sinfg::Vector),sizeof(sinfg::Color),sizeof(sinfg::Canvas),sizeof(sinfg::Layer))

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

//! Version checker \internal
/*! Checks to make sure that the library
**	version matches with what the program
**	was compiled against.
**	\see SINFG_CHECK_VERSION()
*/
extern bool check_version_(int v,int vec_size, int color_size,int canvas_size,int layer_size);

extern const char *get_version();

extern const char *get_build_date();

extern const char *get_build_time();

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
