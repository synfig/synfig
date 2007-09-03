/* === S Y N F I G ========================================================= */
/*!	\file layer_pastecanvas.h
**	\brief Template Header
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

	etl::loose_handle<synfig::Canvas> canvas;

	//! Recursion depth counter
	mutable int depth;

	Real zoom;

	Time time_offset;

	mutable Time curr_time;

	bool optimize_layers(synfig::Context context,synfig::CanvasBase&)const;

	bool muck_with_time_;

	bool children_lock;

	mutable Rect bounds;

	sigc::connection child_changed_connection;
public:

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
