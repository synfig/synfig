/* === S Y N F I G ========================================================= */
/*!	\file layer_pastecanvas.h
**	\brief Header file for implementation of the "Paste Canvas" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

class Layer_PasteCanvas : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:

	Vector origin;
	Vector focus;
	Vector junk;

	etl::loose_handle<synfig::Canvas> canvas;

	//! Recursion depth counter
	mutable int depth;

	Real zoom;

	Time time_offset;

	mutable Time curr_time;

	bool muck_with_time_;

	bool children_lock;

	mutable Rect bounds;

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

	sigc::signal<void> signal_subcanvas_changed_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	sigc::signal<void>& signal_subcanvas_changed() { return signal_subcanvas_changed_; }

public:

	void update_renddesc();

	virtual void on_canvas_set();

	void set_muck_with_time(bool x=false) { muck_with_time_=x; }

	etl::handle<synfig::Canvas> get_sub_canvas()const { return canvas; }
	void set_sub_canvas(etl::handle<synfig::Canvas> x);

	Real get_zoom()const { return zoom; }

	Time get_time_offset()const { return time_offset; }

	Point get_origin()const { return origin; }

	Layer_PasteCanvas();
	virtual ~Layer_PasteCanvas();

	virtual String get_local_name()const;

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual void set_time(Context context, Time time)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

	virtual synfig::Rect get_bounding_rect()const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

protected:
	virtual void get_times_vfunc(Node::time_set &set) const;

}; // END of class Layer_PasteCanvas

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
