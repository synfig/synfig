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

#ifndef __SYNFIG_LAYER_PASTEIMAGE_H
#define __SYNFIG_LAYER_PASTEIMAGE_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include "color.h"
#include "vector.h"
#include "real.h"
#include "time.h"
#include "canvasbase.h"
#include "canvas.h"
#include "rect.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
/*!	\class Layer_PasteCanvas
**	\brief Class of the Pasted Canvas layer.
*/
class Layer_PasteCanvas : public Layer_Composite, public Layer_NoDeform
{
	//! Layer module: defines the needed members to belong to a layer's factory.
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: (Vector) Origin of the paste canvas layer
	ValueBase param_origin;
	//! Parameter: (Vector) Focus of the zoom of the paste canvas layer
	ValueBase param_focus;
	//! Parameter: (etl::loose_handle<synfig::Canvas>) The canvas parameter
	etl::loose_handle<synfig::Canvas> canvas;
	//! Parameter: (Real) Zoom of the paste canvas layer
	ValueBase param_zoom;
	//! Parameter: (Time) Time offset of the paste canvas layer
	ValueBase param_time_offset;
	//! Parameter: (Real) The value to grow the children outline layers
	ValueBase param_outline_grow;
	//! Parameter: (bool) Value that avoid hit check to go depth into the children.
	ValueBase param_children_lock;
	//! Parameter: Current time of the paste canvas layer. \see set_time
	mutable ValueBase param_curr_time;
	//! Parameter: (bool) Z_Depth Range is active
	ValueBase param_z_range;
	//! Parameter: (Real) Z_Depth Range position
	ValueBase param_z_range_position;
	//! Parameter: (Real) Z_Depth Range depth
	ValueBase param_z_range_depth;
	//! Parameter: (Real) Z_Depth Range transition
	ValueBase param_z_range_blur;

	//! \todo writeme!
	bool muck_with_time_;
	//! Recursion depth counter.
	mutable int depth;

	//! Boundaries of the paste canvas layer. It is the canvas's boundary
	//! affected by the zoom, origin and focus.
	mutable Rect bounds;
	//! signal connection for children. Seems to be used only here
	sigc::connection child_changed_connection;

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
	//! Sets muck_with_time.
	//! \todo writeme!
	void set_muck_with_time(bool x=false) { muck_with_time_=x; }

	//! Gets the canvas parameter. It is called sub_canvas to avoid confusion
	//! with the get_canvas from the Layer class.
	etl::handle<synfig::Canvas> get_sub_canvas()const { return canvas; }
	//! Sets the canvas parameter.
	//! \see get_sub_canvas()
	void set_sub_canvas(etl::handle<synfig::Canvas> x);
	//! Gets zoom parameter
	Real get_zoom()const { return param_zoom.get(Real()); }
	//! Gets time offset parameter
	Time get_time_offset()const { return param_time_offset.get(Time()); }
	//! Get origin parameter
	Point get_origin()const { return param_origin.get(Vector()); }
	//! Get focus parameter
	Vector get_focus()const { return param_focus.get(Vector()); }
	//! Default constructor
	Layer_PasteCanvas();
	//! Destructor
	virtual ~Layer_PasteCanvas();
	//! Returns a string with the localized name of this layer
	virtual String get_local_name()const;
	//!	Sets the parameter described by \a param to \a value. \see Layer::set_param
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	//! Get the value of the specified parameter. \see Layer::get_param
	virtual ValueBase get_param(const String & param)const;
	//! Gets the blend color of the Layer in the context at \a pos
	virtual Color get_color(Context context, const Point &pos)const;
	//! Sets the time of the Paste Canvas Layer and those under it
	virtual void set_time(IndependentContext context, Time time)const;
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

protected:
	//!	Function to be overloaded that fills the Time Point Set with
	//! all the children Time Points. In this case the children Time Points
	//! are the canvas parameter children layers Time points and the Paste Canvas
	//! Layer time points. \todo clarify all this comments.
	virtual void get_times_vfunc(Node::time_set &set) const;

}; // END of class Layer_PasteCanvas

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
