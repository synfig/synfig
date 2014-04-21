/* === S Y N F I G ========================================================= */
/*!	\file skeletondeformationentry.h
**	\brief SkeletonDeformationEntryClass Class
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_SKELETONDEFORMATIONENTRY_H
#define __SYNFIG_SKELETONDEFORMATIONENTRY_H

/* === H E A D E R S ======================================================= */

#include "vector.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class SkeletonDeformationEntry
{
public:
	Real r0, r1;
	Vector initial_p0;
	Vector initial_p1;
	Vector current_p0;
	Vector current_p1;
	inline SkeletonDeformationEntry(): r0(0.0), r1(0.0) { }
}; // END of class SkeletonDeformationEntry

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
