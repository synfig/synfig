/* === S I N F G =========================================================== */
/*!	\file layer_pastecanvas.h
**	\brief Template Header
**
**	$Id: layer_pastecanvas.h,v 1.2 2005/01/24 03:08:18 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_LAYER_PASTEIMAGE_H
#define __SINFG_LAYER_PASTEIMAGE_H

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

namespace sinfg {
	
class Layer_PasteCanvas : public Layer_Composite, public Layer_NoDeform
{
	SINFG_LAYER_MODULE_EXT
private:

	Vector origin;

	etl::loose_handle<sinfg::Canvas> canvas;

	//! Recursion depth counter
	mutable int depth;

	Real zoom;

	Time time_offset;

	mutable Time curr_time;

	bool optimize_layers(sinfg::Context context,sinfg::CanvasBase&)const;

	bool do_not_muck_with_time_;
	
	bool children_lock;

	mutable Rect bounds;

	sigc::connection child_changed_connection;
public:

	virtual void on_canvas_set();

	void set_do_not_muck_with_time(bool x=true) { do_not_muck_with_time_=true; }

	etl::handle<sinfg::Canvas> get_sub_canvas()const { return canvas; }
	void set_sub_canvas(etl::handle<sinfg::Canvas> x);
	
	Real get_zoom()const { return zoom; }

	Time get_time_offset()const { return time_offset; }

	Point get_origin()const { return origin; }

	Layer_PasteCanvas();
	virtual ~Layer_PasteCanvas();

	virtual String get_local_name()const;
	
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual void set_time(Context context, Time time)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

	virtual sinfg::Rect get_bounding_rect()const;

	virtual sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;
	
protected:
	virtual void get_times_vfunc(Node::time_set &set) const;	

}; // END of class Layer_PasteCanvas

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
