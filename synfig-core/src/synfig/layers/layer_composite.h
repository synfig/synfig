/* === S Y N F I G ========================================================= */
/*!	\file layer_composite.h
**	\brief Composite Layer Class Implementation
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_LAYER_COMPOSITE_H
#define __SYNFIG_LAYER_COMPOSITE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Layer_NoDeform {};


/*!	\class Layer_Composite
**	\brief Base class for layers that put stuff on top of lower layers
*/
class Layer_Composite : public Layer
{
private:
	//! The amount of composite
	ValueBase param_amount;
	//! The blend method for the composition
	ValueBase param_blend_method;

protected:
	//! Default constructor. Not used directly.
	explicit Layer_Composite(Real amount=1.0, Color::BlendMethod blend_method=Color::BLEND_COMPOSITE);

	//! Converted blend is used to check if an old version of canvas
	//! is used in the composition. Old Straight was used as new Composite
	//! \todo verify this
	bool converted_blend_;
	//! Transparent color is used for old canvas versions.
	//!Old Straight plus transparent color seems to be the same new than alpha over.
	bool transparent_color_;

	/**
	 * Convenient method to help to implement hit_check() for
	 * Layer_Composite -based layers.
	 *
	 * It keep us from falling into the trap some blend methods may set.
	 *
	 * It's meant to be called in hit_check() implementations only.
	 *
	 * @param context Next layer down in canvas @see Layer::hit_check()
	 * @param point Point to check if this layer is hittable
	 * @param[out] check_myself_first If true, the caller layer must check
	 *              if @a point is inside the layer. If so, return the layer itself.
	 *              If not, return the context.hit_check()
	 * @return The handle of the layer under @a point. If there is not
	 *				a layer under @a point, then returns an empty handle.
	 */
	Layer::Handle basic_hit_check(synfig::Context context, const synfig::Point &point, bool& check_myself_first) const;

public:
	//! Gets the amount of the layer
	float get_amount()const { return param_amount.get(Real()); }
	//! Sets the amount of the layer and returns this layer
	Layer_Composite& set_amount(float x) { param_amount.set(x); return *this; }
	//! Gets the blend method of the layer
	Color::BlendMethod get_blend_method()const { return Color::BlendMethod((param_blend_method.get(int()))); }
	//! Sets the blend method of the layer and returns this layer
	Layer_Composite& set_blend_method(Color::BlendMethod x) { param_blend_method.set(int(x)); return *this; }
	//! Returns true is amount is 1 and blend method is straight
	virtual bool is_solid_color()const { return param_amount.get(Real())==1.0f && param_blend_method.get(int())==Color::BLEND_STRAIGHT; }
	//! Returns true if the amount is zero.
	bool is_disabled()const { return param_amount.get(Real())==0.0f; }
	//! Gets the parameter vocabulary. To be overridden by the derived.
	virtual Vocab get_param_vocab()const;
	//! Sets the value for the given parameter.
	virtual bool set_param(const String &param, const ValueBase &value);
	//! Gets the value of the given parameter
	virtual ValueBase get_param(const String &param)const;
	//!Returns the rectangle that includes the context of the layer and
	//! the intersection of the layer in case it is active and not onto
	virtual Rect get_full_bounding_rect(Context context)const;

protected:
	virtual rendering::Task::Handle build_composite_task_vfunc(ContextParams context_params)const;
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;
}; // END of class Layer_Composite

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
