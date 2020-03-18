/* === S Y N F I G ========================================================= */
/*!	\file action_param.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */
#include"global_variables.h"



/* === G L O B A L  V A R I A B L E S ======================================================= */

const double global_variables::Polyg_eps_max = 1;     // Sequence simplification max error
const double global_variables::Polyg_eps_mul = 0.75;  // Sequence simple thickness-multiplier error
const double global_variables::Quad_eps_max = 1000000;  // As in centerlinetostrokes.cpp, for sequence conversion into strokes
float global_variables::unit_size;
float global_variables::h_factor = 1;
float global_variables::w_factor = 1;
bool global_variables::max_thickness_zero = false;

