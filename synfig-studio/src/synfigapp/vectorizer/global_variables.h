/* === S Y N F I G ========================================================= */
/*!	\file global _variable.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_APP_GLOBAL_VARIABLE_H
#define __SYNFIG_APP_GLOBAL_VARIABL_H

/* === C L A S S E S & S T R U C T S ======================================= */

class global_variables {
  public:
    static const double Polyg_eps_max;     // Sequence simplification max error
    static const double Polyg_eps_mul;  // Sequence simple thickness-multiplier error
    static const double Quad_eps_max;  // As in centerlinetostrokes.cpp, for sequence conversion into strokes
    static float unit_size;
    static float h_factor;
    static float w_factor;
    static bool max_thickness_zero;
};

#endif
