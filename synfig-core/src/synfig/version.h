/* === S Y N F I G ========================================================= */
/*!	\file version.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VERSION_H
#define __SYNFIG_VERSION_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/*! \def SYNFIG_VERSION
**	\brief Synfig API Version
**
**	The macro SYNFIG_VERSION can be set to ensure
**	compile-time compatibility with future versions
**	of Synfig. The first two digits are the major
**	version, the second two digits are the minor
**	version, and the last two digits are the
**	revision release.
*/
#ifndef SYNFIG_VERSION
//#define SYNFIG_VERSION (010200)
#define SYNFIG_VERSION ("01.02.00")
#endif

/*!	Increment this value whenever
**	the library changes in a way
**	that breaks library compatibility
*/
#define SYNFIG_LIBRARY_VERSION	50

/*! \writeme */
#define SYNFIG_CHECK_VERSION()	synfig::check_version_(SYNFIG_LIBRARY_VERSION,sizeof(synfig::Vector),sizeof(synfig::Color),sizeof(synfig::Canvas),sizeof(synfig::Layer))

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//! Version checker \internal
/*! Checks to make sure that the library
**	version matches with what the program
**	was compiled against.
**	\see SYNFIG_CHECK_VERSION()
*/
extern bool check_version_(size_t v, size_t vec_size, size_t color_size, size_t canvas_size, size_t layer_size);

extern const char *get_version();

extern const char *get_build_date();

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
