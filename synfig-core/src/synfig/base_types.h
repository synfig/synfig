/* === S Y N F I G ========================================================= */
/*!	\file base_types.h
**	\brief Template Header
**
**	\legal
**	......... ... 2014 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_BASE_TYPES_H
#define __SYNFIG_BASE_TYPES_H

/* === H E A D E R S ======================================================= */

#include "type.h"
#include "synfig_export.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
	SYNFIG_EXPORT extern Type &type_bool;
	SYNFIG_EXPORT extern Type &type_integer;
	SYNFIG_EXPORT extern Type &type_angle;
	SYNFIG_EXPORT extern Type &type_time;
	SYNFIG_EXPORT extern Type &type_real;
	SYNFIG_EXPORT extern Type &type_vector;
	SYNFIG_EXPORT extern Type &type_color;
	SYNFIG_EXPORT extern Type &type_segment;
	SYNFIG_EXPORT extern Type &type_bline_point;
	SYNFIG_EXPORT extern Type &type_matrix;
	SYNFIG_EXPORT extern Type &type_bone_weight_pair;
	SYNFIG_EXPORT extern Type &type_width_point;
	SYNFIG_EXPORT extern Type &type_dash_item;
	SYNFIG_EXPORT extern Type &type_list;
	SYNFIG_EXPORT extern Type &type_canvas;
	SYNFIG_EXPORT extern Type &type_string;
	SYNFIG_EXPORT extern Type &type_gradient;
	SYNFIG_EXPORT extern Type &type_bone_object;
	SYNFIG_EXPORT extern Type &type_bone_valuenode;
	SYNFIG_EXPORT extern Type &type_transformation;
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
