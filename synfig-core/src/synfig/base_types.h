/* === S Y N F I G ========================================================= */
/*!	\file base_types.h
**	\brief Template Header
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

#ifndef __SYNFIG_BASE_TYPES_H
#define __SYNFIG_BASE_TYPES_H

/* === H E A D E R S ======================================================= */

#include "type.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
	extern Type &type_bool;
	extern Type &type_integer;
	extern Type &type_angle;
	extern Type &type_time;
	extern Type &type_real;
	extern Type &type_vector;
	extern Type &type_color;
	extern Type &type_segment;
	extern Type &type_bline_point;
	extern Type &type_matrix;
	extern Type &type_bone_weight_pair;
	extern Type &type_width_point;
	extern Type &type_dash_item;
	extern Type &type_list;
	extern Type &type_canvas;
	extern Type &type_string;
	extern Type &type_gradient;
	extern Type &type_bone_object;
	extern Type &type_bone_valuenode;
	extern Type &type_transformation;
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
