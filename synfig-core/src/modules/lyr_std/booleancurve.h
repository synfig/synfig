/* === S Y N F I G ========================================================= */
/*!	\file booleancurve.h
**	\brief Boolean Curve Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_BOOLEAN_CURVE_H
#define __SYNFIG_BOOLEAN_CURVE_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <synfig/layers/layer_shape.h>
#include <synfig/blinepoint.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class BooleanCurve : public Layer_Shape
{
	//dynamic list of regions and such
	typedef std::vector< std::vector<BLinePoint> >	region_list_type;
	region_list_type	regions;

	enum BOOLEAN_OP
	{
		Union = 0,
		Intersection,
		MutualExclude,
		Num_Boolean_Ops
	};

	//int operation;

public:

	BooleanCurve();
	~BooleanCurve();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;

	virtual Vocab get_param_vocab()const;

	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context, Surface *surface, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
