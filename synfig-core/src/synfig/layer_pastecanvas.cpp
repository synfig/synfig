/* === S Y N F I G ========================================================= */
/*!	\file layer_pastecanvas.cpp
**	\brief Implementation of the "Paste Canvas" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "layer_pastecanvas.h"
#include "string.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "renddesc.h"
#include "surface.h"
#include "value.h"
#include "valuenode.h"
#include "canvas.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define MAX_DEPTH 10

// if this isn't defined, the 'dead heads' in examples/pirates.sifz don't render properly
#define SYNFIG_CLIP_PASTECANVAS

//#ifdef __APPLE__
//#undef SYNFIG_CLIP_PASTECANVAS
//#endif

/* === C L A S S E S ======================================================= */

class depth_counter	// Makes our recursive depth counter exception-safe
{
	int *depth;
public:
	depth_counter(int &x):depth(&x) { (*depth)++; }
	~depth_counter() { (*depth)--; }
};

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_PasteCanvas);
SYNFIG_LAYER_SET_NAME(Layer_PasteCanvas,"PasteCanvas"); // todo: use paste_canvas
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_PasteCanvas,N_("Paste Canvas"));
SYNFIG_LAYER_SET_CATEGORY(Layer_PasteCanvas,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_PasteCanvas,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_PasteCanvas,"$Id$");

/* === M E T H O D S ======================================================= */

Layer_PasteCanvas::Layer_PasteCanvas():
	origin(0,0),
	focus(0,0),
	depth(0),
	zoom(0),
	time_offset(0),
	extra_reference(false)
{
	children_lock=false;
	muck_with_time_=true;
	curr_time=Time::begin();
	outline_grow=0.0;
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
	set_param_static("children_lock", true);
}

Layer_PasteCanvas::~Layer_PasteCanvas()
{
/*	if(canvas)
		canvas->parent_set.erase(this);
*/

	//if(canvas)DEBUGINFO(strprintf("%d",canvas->count()));

	set_sub_canvas(0);

	//if(canvas && (canvas->is_inline() || !get_canvas() || get_canvas()->get_root()!=canvas->get_root()))
	//if(extra_reference)
	//	canvas->unref();
}

String
Layer_PasteCanvas::get_local_name()const
{
	if(!canvas)	return _("Group");
	if(canvas->is_inline()) return _("Group");
	if(canvas->get_root()==get_canvas()->get_root()) return '[' + canvas->get_id() + ']';

	return '[' + canvas->get_file_name() + ']';
}

Layer::Vocab
Layer_PasteCanvas::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Point where you want the origin to be"))
	);
	ret.push_back(ParamDesc("canvas")
		.set_local_name(_("Canvas"))
		.set_description(_("Canvas to paste"))
	);
	ret.push_back(ParamDesc("zoom")
		.set_local_name(_("Zoom"))
		.set_description(_("Size of canvas"))
	);

	ret.push_back(ParamDesc("time_offset")
		.set_local_name(_("Time Offset"))
		.set_description(_("Time Offset to apply to the context"))
	);

	ret.push_back(ParamDesc("children_lock")
		.set_local_name(_("Children Lock"))
		.set_description(_("When checked prevents to select the children using the mouse click"))
	);

	ret.push_back(ParamDesc("focus")
		.set_local_name(_("Focus Point"))
		.set_origin("origin")
		.set_connect("origin")
		.set_description(_("Point to remain fixed when zooming"))
	//	.set_invisible_duck()
	);

	ret.push_back(ParamDesc("outline_grow")
		.set_local_name(_("Outline Grow"))
		.set_description(_("Exponential value to grow children Outline layers width"))
	);

	if(canvas && !(canvas->is_inline()))
	{
		ret.back().hidden();
	}

	// optimize_layers() in canvas.cpp makes a new PasteCanvas layer
	// and copies over the parameters of the old layer.  the
	// 'curr_time' member wasn't being copied, so I've added it as a
	// hidden, non critical parameter, and now it will be.  this
	// allows a single exported subcanvas to be used more than once at
	// a time, with different time offets in each.  see bug #1896557.
	ret.push_back(ParamDesc("curr_time")
		.set_local_name(_("Current Time"))
		.not_critical()
		.hidden()
	);

	return ret;
}

bool
Layer_PasteCanvas::set_param(const String & param, const ValueBase &value)
{
	IMPORT(origin);
	IMPORT(focus);

	// IMPORT(canvas);
	if(param=="canvas" && value.same_type_as(Canvas::Handle()))
	{
		set_sub_canvas(value.get(Canvas::Handle()));
		set_param_static(param, value.get_static());
		return true;
	}
	//! \todo this introduces bug 1844764 if enabled; it was introduced in r954.
	// http://synfig.org/images/3/3d/Moving-waypoints.sifz is an
	// example of an animation that has its waypoints displayed
	// incorrectly without this fix; select the outer layer and drag
	// the time slider.  The linear waypoints don't take effect until
	// 5s, but the time slider appears to pass the first one at 3s.
#if 0
	if (param=="time_offset" && value.same_type_as(time_offset))
	{
		if (time_offset != value.get(Time()))
		{
			value.put(&time_offset);
			// notify that the time_offset has changed so we can update the
			// waypoint positions in parent layers
			changed();
		}
		return true;
	}
#else
	IMPORT(time_offset);
#endif

	IMPORT(children_lock);
	IMPORT(zoom);
	IMPORT(curr_time);
	IMPORT(outline_grow);
	return Layer_Composite::set_param(param,value);
}

void
Layer_PasteCanvas::set_sub_canvas(etl::handle<synfig::Canvas> x)
{
	if(canvas && muck_with_time_)
		remove_child(canvas.get());

	// if(canvas && (canvas->is_inline() || !get_canvas() || get_canvas()->get_root()!=canvas->get_root()))
	if (extra_reference)
		canvas->unref();

	child_changed_connection.disconnect();

	if (canvas != x) signal_subcanvas_changed()();

	canvas=x;

	/*if(canvas)
		child_changed_connection=canvas->signal_changed().connect(
			sigc::mem_fun(
				*this,
				&Layer_PasteCanvas::changed
			)
		);
	*/
	if(canvas)
		bounds = ((canvas->get_context().get_full_bounding_rect() - focus) * exp(zoom) + origin + focus);

	if(canvas && muck_with_time_)
		add_child(canvas.get());

	if(canvas && (canvas->is_inline() || !get_canvas() || get_canvas()->get_root()!=canvas->get_root()))
	{
		canvas->ref();
		extra_reference = true;
	}
	else
		extra_reference = false;

	if(canvas)
		on_canvas_set();
}

// when a pastecanvas that contains another pastecanvas is copy/pasted
// from one document to another, only the outermost pastecanvas was
// getting its renddesc set to match that of its new parent.  this
// function is used to recurse through the pastecanvas copying its
// renddesc to any pastecanvases it contains (bug #2116947, svn r2200)
void
Layer_PasteCanvas::update_renddesc()
{
	if(!get_canvas() || !canvas || !canvas->is_inline()) return;

	canvas->rend_desc()=get_canvas()->rend_desc();
	for (Context context = canvas->get_context(); !context->empty(); context++)
	{
		etl::handle<Layer_PasteCanvas> paste = etl::handle<Layer_PasteCanvas>::cast_dynamic(*context);
		if (paste) paste->update_renddesc();
	}
}

// This is called whenever the parent canvas gets set/changed
void
Layer_PasteCanvas::on_canvas_set()
{
	if(get_canvas() && canvas && canvas->is_inline() && canvas->parent()!=get_canvas())
	{
		canvas->set_inline(get_canvas());
	}
}

ValueBase
Layer_PasteCanvas::get_param(const String& param)const
{
	EXPORT(origin);
	EXPORT(focus);
	EXPORT(canvas);
	EXPORT(zoom);
	EXPORT(time_offset);
	EXPORT(children_lock);
	EXPORT(curr_time);
	EXPORT(outline_grow);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

void
Layer_PasteCanvas::set_time(Context context, Time time)const
{
	if(depth==MAX_DEPTH)return;depth_counter counter(depth);
	curr_time=time;

	context.set_time(time);
	if(canvas)
	{
		canvas->set_time(time+time_offset);

		bounds=(canvas->get_context().get_full_bounding_rect()-focus)*exp(zoom)+origin+focus;
	}
	else
		bounds=Rect::zero();
}

synfig::Layer::Handle
Layer_PasteCanvas::hit_check(synfig::Context context, const synfig::Point &pos)const
{
	if(depth==MAX_DEPTH)return 0;depth_counter counter(depth);

	if (canvas) {
		Point target_pos=(pos-focus-origin)/exp(zoom)+focus;

		if(canvas && get_amount() && canvas->get_context().get_color(target_pos).get_a()>=0.25)
		{
			if(!children_lock)
			{
				return canvas->get_context().hit_check(target_pos);
			}
			return const_cast<Layer_PasteCanvas*>(this);
		}
	}
	return context.hit_check(pos);
}

Color
Layer_PasteCanvas::get_color(Context context, const Point &pos)const
{
	if(!canvas || !get_amount())
		return context.get_color(pos);

	if(depth==MAX_DEPTH)return Color::alpha();depth_counter counter(depth);

	Point target_pos=(pos-focus-origin)/exp(zoom)+focus;

	return Color::blend(canvas->get_context().get_color(target_pos),context.get_color(pos),get_amount(),get_blend_method());
}


bool
Layer_PasteCanvas::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if(cb && !cb->amount_complete(0,10000)) return false;

	if(depth==MAX_DEPTH)
		// if we are at the extent of our depth,
		// then we should just return whatever is under us.
		return context.accelerated_render(surface,quality,renddesc,cb);

	depth_counter counter(depth);

	if(!canvas || !get_amount())
		return context.accelerated_render(surface,quality,renddesc,cb);

	SuperCallback stageone(cb,0,4500,10000);
	SuperCallback stagetwo(cb,4500,9000,10000);
	SuperCallback stagethree(cb,9000,9999,10000);

	RendDesc desc(renddesc);
	Vector::value_type zoomfactor=1.0/exp(zoom);
	desc.clear_flags();
	desc.set_tl((desc.get_tl()-focus-origin)*zoomfactor+focus);
	desc.set_br((desc.get_br()-focus-origin)*zoomfactor+focus);
	desc.set_flags(RendDesc::PX_ASPECT);

	if (is_solid_color() || context->empty())
	{
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		surface->clear();
	}
	else if (!context.accelerated_render(surface,quality,renddesc,&stageone))
		return false;

	Real grow_value(get_parent_canvas_grow_value());
	canvas->set_grow_value(outline_grow+grow_value);

	if(muck_with_time_ && curr_time!=Time::begin() /*&& canvas->get_time()!=curr_time+time_offset*/)
		canvas->set_time(curr_time+time_offset);

	Color::BlendMethod blend_method(get_blend_method());
	const Rect full_bounding_rect(canvas->get_context().get_full_bounding_rect());

	bool blend_using_straight = false; // use 'straight' just for the central blit

	// sometimes the user changes the parameters while we're
	// rendering, causing our pasted canvas' bounding box to shrink
	// and no longer overlap with our tile.  if that has happened,
	// let's just stop now - we'll be refreshing soon anyway
	//! \todo shouldn't a mutex ensure this isn't needed?
	// http://synfig.org/images/d/d2/Bbox-change.sifz is an example
	// that shows this happening - open the encapsulation, select the
	// 'shade', and toggle the 'invert' parameter quickly.
	// Occasionally you'll see:
	//   error: Context::accelerated_render(): Layer "shade" threw a bad_alloc exception!
	// where the shade layer tries to allocate itself a canvas of
	// negative proportions, due to changing bounding boxes.
	if (!etl::intersect(desc.get_rect(), full_bounding_rect))
	{
		warning("%s:%d bounding box shrank while rendering?", __FILE__, __LINE__);
		return true;
	}

	// we have rendered what's under us, if necessary
	if(context->empty())
	{
		// if there's nothing under us, and we're blending 'onto', then we've finished
		if (Color::is_onto(blend_method)) return true;

		// there's nothing under us, so using straight blending is
		// faster than and equivalent to using composite, but we don't
		// want to blank the surrounding areas
		if (blend_method==Color::BLEND_COMPOSITE) blend_using_straight = true;
	}

	if (!etl::intersect(context.get_full_bounding_rect(),(full_bounding_rect-focus)*exp(zoom)+origin+focus))
	{
		// if there's no intersection between the context and our
		// surface, and we're rendering 'onto', then we're done
		if (Color::is_onto(blend_method) && !Color::is_straight(blend_method))
			return true;

		/* 'straight' is faster than 'composite' and has the same
		 * effect if the affected area of the lower layer is
		 * transparent;  however, if we're not clipping the blit to
		 * just the bounding rectangle, the affected area is the whole
		 * tile, so we can't use this optimisation.  if we are
		 * clipping, then we can use 'straight' to blit the clipped
		 * rectangle, but we shouldn't set blend_method to 'straight',
		 * or the surrounding areas will be blanked, which we don't
		 * want.
		 */
#ifdef SYNFIG_CLIP_PASTECANVAS
		if (blend_method==Color::BLEND_COMPOSITE) blend_using_straight = true;
#endif	// SYNFIG_CLIP_PASTECANVAS
	}

#ifdef SYNFIG_CLIP_PASTECANVAS
	Rect area(desc.get_rect() & full_bounding_rect);

	Point min(area.get_min());
	Point max(area.get_max());

	if (desc.get_tl()[0] > desc.get_br()[0]) swap(min[0], max[0]);
	if (desc.get_tl()[1] > desc.get_br()[1]) swap(min[1], max[1]);

	const int x(floor_to_int((min[0] - desc.get_tl()[0]) / desc.get_pw()));
	const int y(floor_to_int((min[1] - desc.get_tl()[1]) / desc.get_ph()));
	const int w( ceil_to_int((max[0] - desc.get_tl()[0]) / desc.get_pw()) - x);
	const int h( ceil_to_int((max[1] - desc.get_tl()[1]) / desc.get_ph()) - y);

	const int tw = desc.get_w();
	const int th = desc.get_h();

	desc.set_subwindow(x,y,w,h);

	// \todo this used to also have "area.area()<=0.000001 || " - is it useful?
	//		 it was causing bug #1809480 (Zoom in beyond 8.75 in nested canvases fails)
	if(desc.get_w()==0 || desc.get_h()==0)
	{
		if(cb && !cb->amount_complete(10000,10000)) return false;
		return true;
	}

	// SYNFIG_CLIP_PASTECANVAS is defined, so we are only touching the
	// pixels within the affected rectangle.  If the blend method is
	// 'straight', then we need to blend transparent pixels with the
	// clipped areas of this tile, because with the 'straight' blend
	// method, even transparent pixels have an effect on the layers below
	if (Color::is_straight(blend_method))
	{
		Surface clearsurface;

		Surface::alpha_pen apen(surface->begin());
		apen.set_alpha(get_amount());

		// the area we're about to blit is transparent, so it doesn't
		// matter whether we use 'straight' or 'straight onto' here
		if (blend_method == Color::BLEND_ALPHA_BRIGHTEN)
			apen.set_blend_method(blend_method);
		else
			apen.set_blend_method(Color::BLEND_STRAIGHT);

		/* This represents the area we're pasting into the tile,
		 * within the tile as a whole.	Areas (A), (B), (C) and (D)
		 * need blending with the underlying context if they're not
		 * zero-sized:
		 *
		 *		 0	   x		 x+w	  tw
		 *	 0	 +------------------------+
		 *		 |						  |
		 *		 |			(A)			  |
		 *		 |						  |
		 *	 y	 | - - +----------+ - - - |
		 *		 |	   |		  |		  |
		 *		 | (C) |  w by h  |	 (D)  |
		 *		 |	   |		  |		  |
		 *	 y+h | - - +----------+ - - - |
		 *		 |						  |
		 *		 |			(B)			  |
		 *		 |						  |
		 *	 tw	 +------------------------+
		 */

		if (y > 0)				// draw the full-width strip above the rectangle (A)
		{ apen.move_to(0,0);   clearsurface.set_wh(tw,y);        clearsurface.clear(); clearsurface.blit_to(apen); }
		if (y+h < th)			// draw the full-width strip below the rectangle (B)
		{ apen.move_to(0,y+h); clearsurface.set_wh(tw,th-(y+h)); clearsurface.clear(); clearsurface.blit_to(apen); }
		if (x > 0)				// draw the box directly left of the rectangle (C)
		{ apen.move_to(0,y);   clearsurface.set_wh(x,h);         clearsurface.clear(); clearsurface.blit_to(apen); }
		if (x+w < tw)			// draw the box directly right of the rectangle (D)
		{ apen.move_to(x+w,y); clearsurface.set_wh(tw-(x+w),h);  clearsurface.clear(); clearsurface.blit_to(apen); }
	}
#endif	// SYNFIG_CLIP_PASTECANVAS

	// render the canvas to be pasted onto pastesurface
	Surface pastesurface;
	if(!canvas->get_context().accelerated_render(&pastesurface,quality,desc,&stagetwo))
		return false;

#ifdef SYNFIG_CLIP_PASTECANVAS
	Surface::alpha_pen apen(surface->get_pen(x,y));
#else  // SYNFIG_CLIP_PASTECANVAS
	Surface::alpha_pen apen(surface->begin());
#endif	// SYNFIG_CLIP_PASTECANVAS

	apen.set_alpha(get_amount());
	apen.set_blend_method(blend_using_straight ? Color::BLEND_STRAIGHT : blend_method);
	pastesurface.blit_to(apen);

	if(cb && !cb->amount_complete(10000,10000)) return false;

	return true;
}

Rect
Layer_PasteCanvas::get_bounding_rect()const
{
	return bounds;
}

void Layer_PasteCanvas::get_times_vfunc(Node::time_set &set) const
{
	Node::time_set tset;
	if(canvas) tset = canvas->get_times();

	Node::time_set::iterator i = tset.begin(), end = tset.end();

	//Make sure we offset the time...
	//! \todo: SOMETHING STILL HAS TO BE DONE WITH THE OTHER DIRECTION
	//		   (recursing down the tree needs to take this into account too...)
	for(; i != end; ++i)
		set.insert(*i
#ifdef ADJUST_WAYPOINTS_FOR_TIME_OFFSET // see node.h
				   - time_offset
#endif
			);

	Layer::get_times_vfunc(set);
}


bool
Layer_PasteCanvas::set_param_static(const String &param, const bool x)
{
	return Layer_Composite::set_param_static(param, x);
}


bool
Layer_PasteCanvas::get_param_static(const String &param) const
{
	return Layer_Composite::get_param_static(param);
}

