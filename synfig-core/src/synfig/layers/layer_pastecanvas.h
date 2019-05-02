/* === S Y N F I G ========================================================= */
/*!	\file layer_pastecanvas.h
**	\brief Header file for implementation of the "Paste Canvas" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**	......... ... 2014-2017 Ivan Mahonin
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

#ifndef __SYNFIG_LAYER_PASTECANVAS_H
#define __SYNFIG_LAYER_PASTECANVAS_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/real.h>
#include <synfig/time.h>
#include <synfig/canvasbase.h>
#include <synfig/canvas.h>
#include <synfig/rect.h>
#include <synfig/transformation.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
/*!	\class Layer_PasteCanvas
**	\brief Class of the Pasted Canvas layer.
*/
class Layer_PasteCanvas : public Layer_Composite, public Layer_NoDeform
{
public:
	typedef etl::handle<Layer_PasteCanvas> Handle;

private:
	//! Parameter: (Origin) Position offset
	ValueBase param_origin;
	//! Parameter: (Transformation) Position, rotation and scale of the paste canvas layer
	ValueBase param_transformation;
	//! Parameter: (etl::loose_handle<synfig::Canvas>) The canvas parameter
	etl::loose_handle<synfig::Canvas> sub_canvas;
	//! Parameter: (Real) Time dilation of the paste canvas layer
	ValueBase param_time_dilation;
	//! Parameter: (Time) Time offset of the paste canvas layer
	ValueBase param_time_offset;
	//! Parameter: (Real) The value to grow the children outline layers
	ValueBase param_outline_grow;
	//! Parameter: (bool) Value that avoid hit check to go depth into the children.
	ValueBase param_children_lock;

	//! Recursion depth counter.
	mutable int depth;

	//! Boundaries of the paste canvas layer. It is the canvas's boundary
	//! affected by the origin and transformation.
	mutable Rect bounds;
	//! signal connection for children. Seems to be used only here
	sigc::connection childs_changed_connection;

	// Nasty hack: Remember whether we called an extra ref() when
	// setting the canvas, so we know whether to call an extra unref()
	// when finished with the canvas.
	//
	// Here's the story:
	//
	// The root canvas is destructed first.  That sets the
	// Layer::canvas_ (the parent canvas) of any PasteCanvas layer it
	// contains to nil, due to a call to Layer::set_canvas(0),
	// triggered by the connection made when Layer::set_canvas
	// originally set its canvas_ member to point to the root canvas.
	// ~Canvas does begin_delete() which triggers that connection.
	//
	// After ~Canvas has run, the members of the root canvas are
	// freed, including its children_ list.  If this was the last
	// reference to the child canvas that the pastecanvas uses, that
	// child canvas will Layer_PasteCanvas::set_sub_canvas(0) on the
	// PasteCanvas layer to set its canvas (the child, pasted canvas)
	// not to refer to the soon-to-be destroys child canvas.  But
	// set_sub_canvas() originally looked at the value of
	// Layer::canvas_ (the parent canvas, obtained via
	// Layer::get_canvas()) to decide whether to do an extra ref() on
	// canvas (the child canvas).  We need to unref() it now if we
	// did, but we've forgotten whether we did.  So we use this
	// 'extra_reference' member to store that decision.
	bool extra_reference;

	void childs_changed();

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:
	//! Signal used when the canvas parameter has changed. \see layertreestore.cpp
	sigc::signal<void> signal_subcanvas_changed_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:
	//! Wrapper for the subcanvas changed signal
	sigc::signal<void>& signal_subcanvas_changed() { return signal_subcanvas_changed_; }

public:

	//! Recursively update the Render Description for the inner inline only pasted canvases.
	//! Used for copy and paste Paste Canvas Layers between compositions.
	void update_renddesc();

	//! Every time the Paste Canvas Layer parent canvas is changed, this
	//! is called and it sets the parent of the canvas parameter to that canvas
	//! if it is on line
	virtual void on_canvas_set();

	//! Gets the canvas parameter. It is called sub_canvas to avoid confusion
	//! with the get_canvas from the Layer class.
	etl::handle<synfig::Canvas> get_sub_canvas()const { return sub_canvas; }
	//! Sets the canvas parameter.
	//! \see get_sub_canvas()
	void set_sub_canvas(etl::handle<synfig::Canvas> x);
	//! Gets time dilation parameter
	Real get_time_dilation()const { return param_time_dilation.get(Real()); }
	//! Gets time offset parameter
	Time get_time_offset()const { return param_time_offset.get(Time()); }

	//! Get origin parameter
	Point get_origin()const { return param_origin.get(Point()); }
	//! Get transformation parameter
	Transformation get_transformation()const { return param_transformation.get(Transformation()); }
	//! Get summary transformation
	Transformation get_summary_transformation()const
	{
		return get_transformation().transform( Transformation(-get_origin()) );
	}

	//! Default constructor
	explicit Layer_PasteCanvas(Real amount = 1.0, Color::BlendMethod blend_method = Color::BLEND_COMPOSITE);
	//! Destructor
	virtual ~Layer_PasteCanvas();
	//! Returns a string with the localized name of this layer
	virtual String get_local_name()const;
	//!	Sets the parameter described by \a param to \a value. \see Layer::set_param
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	//! Get the value of the specified parameter. \see Layer::get_param
	virtual ValueBase get_param(const String & param)const;
	//! Sets z_range* fields of specified ContextParams \a cp
	virtual void apply_z_range_to_params(ContextParams &cp)const;
	//! Gets the blend color of the Layer in the context at \a pos
	virtual Color get_color(Context context, const Point &pos)const;

	//! Renders the Canvas to the given Surface in an accelerated manner
	//! See Layer::accelerated_render
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	//! Bounding rect for this layer depends from context_params
	Rect get_bounding_rect_context_dependent(const ContextParams &context_params)const;
	//!Returns the rectangle that includes the context of the layer and
	//! the intersection of the layer in case it is active and not onto
	virtual Rect get_full_bounding_rect(Context context)const;
	//! Gets the parameter vocabulary
	virtual Vocab get_param_vocab()const;
	//! Checks to see if a part of the Paste Canvas Layer is directly under \a point
	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	virtual void set_render_method(Context context, RenderMethod x);

	virtual void fill_sound_processor(SoundProcessor &soundProcessor) const;

	virtual void on_childs_changed() { }

protected:
	virtual Context build_context_queue(Context context, CanvasBase &out_queue)const;

	//! Sets the time of the Paste Canvas Layer and those under it
	virtual void set_time_vfunc(IndependentContext context, Time time)const;
	//! Loads external resources (frames) for child layers of the Paste Canvas Layer
	virtual void load_resources_vfunc(IndependentContext context, Time time)const;
	//! Sets the outline_grow of the Paste Canvas Layer and those under it
	virtual void set_outline_grow_vfunc(IndependentContext context, Real outline_grow)const;
	//!	Function to be overloaded that fills the Time Point Set with
	//! all the children Time Points. In this case the children Time Points
	//! are the canvas parameter children layers Time points and the Paste Canvas
	//! Layer time points. \todo clarify all this comments.
	virtual void get_times_vfunc(Node::time_set &set) const;

	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;
}; // END of class Layer_PasteCanvas

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
