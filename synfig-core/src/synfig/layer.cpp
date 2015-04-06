/* === S Y N F I G ========================================================= */
/*!	\file layer.cpp
**	\brief Layer class implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
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

#include "canvas.h"
#include "layer.h"
#include "render.h"
#include "value.h"
#include <synfig/layers/layer_bitmap.h>
#include <synfig/layers/layer_mime.h>
#include "context.h"
#include "paramdesc.h"
#include "surface.h"

#include <synfig/layers/layer_solidcolor.h>
#include <synfig/layers/layer_polygon.h>
#include <synfig/layers/layer_group.h>
#include <synfig/layers/layer_switch.h>
#include <synfig/layers/layer_motionblur.h>
#include <synfig/layers/layer_duplicate.h>
#include <synfig/layers/layer_skeleton.h>
#include <synfig/layers/layer_skeletondeformation.h>
#include <synfig/layers/layer_sound.h>

#include "valuenodes/valuenode_const.h"

#include "transform.h"
#include "rect.h"
#include "guid.h"

#include <sigc++/adaptors/bind.h>
#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

static Layer::Book* _layer_book;

struct _LayerCounter
{
	static int counter;
	~_LayerCounter()
	{
		if(counter)
			synfig::error("%d layers not yet deleted!",counter);
	}
} _layer_counter;

int _LayerCounter::counter(0);

/* === P R O C E D U R E S ================================================= */

Layer::Book&
Layer::book()
{
	return *_layer_book;
}

void
Layer::register_in_book(const BookEntry &entry)
{
	book()[entry.name]=entry;
}

bool
Layer::subsys_init()
{
	_layer_book=new Book();

#define INCLUDE_LAYER(class)									\
	synfig::Layer::book() [synfig::String(class::name__)] =		\
		BookEntry(class::create,								\
				  class::name__,								\
				  dgettext("synfig", class::local_name__),		\
				  class::category__,							\
				  class::cvs_id__,								\
				  class::version__)

#define LAYER_ALIAS(class,alias)								\
	synfig::Layer::book()[synfig::String(alias)] =				\
		BookEntry(class::create,								\
				  alias,										\
				  alias,										\
				  CATEGORY_DO_NOT_USE,							\
				  class::cvs_id__,								\
				  class::version__)

	INCLUDE_LAYER(Layer_SolidColor);
		LAYER_ALIAS(Layer_SolidColor,	"solid_color");
	INCLUDE_LAYER(Layer_Group);
		LAYER_ALIAS(Layer_Group,		"paste_canvas");
		LAYER_ALIAS(Layer_Group,		"PasteCanvas");
	INCLUDE_LAYER(Layer_Switch);
	INCLUDE_LAYER(Layer_Polygon);
		LAYER_ALIAS(Layer_Polygon,		"Polygon");
	INCLUDE_LAYER(Layer_MotionBlur);
		LAYER_ALIAS(Layer_MotionBlur,	"motion_blur");
	INCLUDE_LAYER(Layer_Duplicate);
	INCLUDE_LAYER(Layer_Skeleton);
	INCLUDE_LAYER(Layer_SkeletonDeformation);
	INCLUDE_LAYER(Layer_Sound);

#undef LAYER_ALIAS
#undef INCLUDE_LAYER

	return true;
}

bool
Layer::subsys_stop()
{
	delete _layer_book;
	return true;
}

/* === M E T H O D S ======================================================= */

Layer::Layer():
	active_(true),
	optimized_(false),
	exclude_from_rendering_(false),
	param_z_depth(Real(0.0f)),
	dirty_time_(Time::end())
{
	_LayerCounter::counter++;
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer::LooseHandle
synfig::Layer::create(const String &name)
{
	if(!book().count(name))
	{
		return Layer::LooseHandle(new Layer_Mime(name));
	}

	Layer* layer(book()[name].factory());
	return Layer::LooseHandle(layer);
}

synfig::Layer::~Layer()
{
	_LayerCounter::counter--;
	while(!dynamic_param_list_.empty())
	{
		remove_child(dynamic_param_list_.begin()->second.get());
		dynamic_param_list_.erase(dynamic_param_list_.begin());
	}

	remove_from_all_groups();

	parent_death_connect_.disconnect();
	begin_delete();
}

void
synfig::Layer::set_canvas(etl::loose_handle<Canvas> x)
{
	if(canvas_!=x)
	{
		parent_death_connect_.disconnect();
		canvas_=x;
		if(x)
		{
			parent_death_connect_=x->signal_deleted().connect(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&Layer::set_canvas
					),
					etl::loose_handle<synfig::Canvas>(0)
				)
			);
		}
		on_canvas_set();
	}
}

void
synfig::Layer::on_canvas_set()
{
}

etl::loose_handle<synfig::Canvas>
synfig::Layer::get_canvas()const
{
	return canvas_;
}

int
Layer::get_depth()const
{
	if(!get_canvas())
		return -1;
	return get_canvas()->get_depth(const_cast<synfig::Layer*>(this));
}

void
Layer::set_active(bool x)
{
	if(active_!=x)
	{
		active_=x;

		Node::on_changed();
		signal_status_changed_();
	}
}

void
Layer::set_exclude_from_rendering(bool x)
{
	if(exclude_from_rendering_!=x)
	{
		exclude_from_rendering_=x;

		Node::on_changed();
		signal_status_changed_();
	}
}

void
Layer::set_description(const String& x)
{
	if(description_!=x)
	{
		description_=x;
		signal_description_changed_();
	}
}

bool
Layer::connect_dynamic_param(const String& param, etl::loose_handle<ValueNode> value_node)
{
	ValueNode::Handle previous(dynamic_param_list_[param]);

	if(previous==value_node)
		return true;

	dynamic_param_list_[param]=ValueNode::Handle(value_node);

	if(previous)
		remove_child(previous.get());

	add_child(value_node.get());

	if(!value_node->is_exported() && get_canvas())
	{
		value_node->set_parent_canvas(get_canvas());
	}

	changed();
	return true;
}

bool
Layer::disconnect_dynamic_param(const String& param)
{
	ValueNode::Handle previous(dynamic_param_list_[param]);

	if(previous)
	{
		dynamic_param_list_.erase(param);

		// fix 2353284: if two parameters in the same layer are
		// connected to the same valuenode and we disconnect one of
		// them, the parent-child relationship for the remaining
		// connection was being deleted.  now we search the parameter
		// list to see if another parameter uses the same valuenode
		DynamicParamList::const_iterator iter;
		for (iter = dynamic_param_list().begin(); iter != dynamic_param_list().end(); iter++)
			if (iter->second == previous)
				break;
		if (iter == dynamic_param_list().end())
			remove_child(previous.get());

		changed();
	}
	return true;
}

void
Layer::on_changed()
{
	if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
		printf("%s:%d Layer::on_changed()\n", __FILE__, __LINE__);

	dirty_time_=Time::end();
	Node::on_changed();
}

bool
Layer::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE(param_z_depth)
	return false;
}

etl::handle<Transform>
Layer::get_transform()const
{
	return 0;
}

float
Layer::get_z_depth(const synfig::Time& t)const
{
	if(!dynamic_param_list().count("z_depth"))
		return param_z_depth.get(Real());
	return (*dynamic_param_list().find("z_depth")->second)(t).get(Real());
}

float
Layer::get_true_z_depth(const synfig::Time& t)const
{
	// TODO: the 1.0001 constant should be somehow user defined
	return get_z_depth(t)*1.0001+get_depth();
}

Layer::Handle
Layer::simple_clone()const
{
	if(!book().count(get_name())) return 0;
	Handle ret = create(get_name()).get();
	ret->group_=group_;
	//ret->set_canvas(get_canvas());
	ret->set_description(get_description());
	ret->set_active(active());
	ret->set_optimized(optimized());
	ret->set_exclude_from_rendering(get_exclude_from_rendering());
	ret->set_param_list(get_param_list());
	for(DynamicParamList::const_iterator iter=dynamic_param_list().begin();iter!=dynamic_param_list().end();++iter)
		ret->connect_dynamic_param(iter->first, iter->second);
	return ret;
}

Layer::Handle
Layer::clone(Canvas::LooseHandle canvas, const GUID& deriv_guid) const
{
	if(!book().count(get_name())) return 0;

	//Layer *ret = book()[get_name()].factory();//create(get_name()).get();
	Handle ret = create(get_name()).get();

	ret->group_=group_;
	//ret->set_canvas(get_canvas());
	ret->set_description(get_description());
	ret->set_active(active());
	ret->set_optimized(optimized());
	ret->set_exclude_from_rendering(get_exclude_from_rendering());
	ret->set_guid(get_guid()^deriv_guid);

	//ret->set_param_list(get_param_list());
	// Process the parameter list so that
	// we can duplicate any inline canvases
	ParamList param_list(get_param_list());
	for(ParamList::const_iterator iter(param_list.begin()); iter != param_list.end(); ++iter)
	{
		if(dynamic_param_list().count(iter->first)==0 && iter->second.get_type()==type_canvas)
		{
			// This parameter is a canvas.  We need a close look.
			Canvas::Handle canvas(iter->second.get(Canvas::Handle()));
			if(canvas && canvas->is_inline())
			{
				// This parameter is an inline canvas! we need to clone it
				// before we set it as a parameter.

				// clone canvas (all code that clones a canvas has this comment)
				Canvas::Handle new_canvas(canvas->clone(deriv_guid));

				ValueBase value(new_canvas);
				ret->set_param(iter->first, value);
				continue;
			}
		}

		// This is a normal parameter,go ahead and set it.
		ret->set_param(iter->first, iter->second);
	}

	// Duplicate the dynamic paramlist, but only the exported data nodes
	DynamicParamList::const_iterator iter;
	for(iter=dynamic_param_list().begin();iter!=dynamic_param_list().end();++iter)
	{
		// Make sure we clone inline canvases
		if(iter->second->get_type()==type_canvas)
		{
			Canvas::Handle canvas((*iter->second)(0).get(Canvas::Handle()));
			if(canvas->is_inline())
			{
				// clone canvas (all code that clones a canvas has this comment)
				Canvas::Handle new_canvas(canvas->clone(deriv_guid));
				ValueBase value(new_canvas);
				ret->connect_dynamic_param(iter->first,ValueNode_Const::create(value));
				continue;
			}
		}

		if(iter->second->is_exported())
			ret->connect_dynamic_param(iter->first,iter->second);
		else
			ret->connect_dynamic_param(iter->first,iter->second->clone(canvas, deriv_guid));
	}

	//ret->set_canvas(0);

	return ret;
}

bool
Layer::reads_context() const
{
	return false;
}

Rect
Layer::get_full_bounding_rect(Context context)const
{
	// No one of overrides of this function don't check the "active" flag
	// So this check also disabled here
	//if(Context::active(context.get_params(),*this))
		return context.get_full_bounding_rect()|get_bounding_rect();
	//return context.get_full_bounding_rect();
}

Rect
Layer::get_bounding_rect()const
{
	return Rect::full_plane();
}

bool
Layer::set_param_list(const ParamList &list)
{
	bool ret=true;
	if(!list.size())
		return false;
	ParamList::const_iterator iter(list.begin());
	for(;iter!=list.end();++iter)
	{
		if(!set_param(iter->first, iter->second))ret=false;
	}
	return ret;
}

Layer::ParamList
Layer::get_param_list()const
{
	ParamList ret;

	Vocab vocab(get_param_vocab());

	Vocab::const_iterator iter=vocab.begin();
	for(;iter!=vocab.end();++iter)
	{
		ret[iter->get_name()]=get_param(iter->get_name());
	}
	return ret;
}

ValueBase
Layer::get_param(const String & param)const
{
	EXPORT_VALUE(param_z_depth);
	return ValueBase();
}

String
Layer::get_version()const
{
	return get_param("version__").get(String());
}

bool
Layer::set_version(const String &/*ver*/)
{
	return false;
}

void
Layer::reset_version()
{
}


void
Layer::set_time(IndependentContext context, Time time)const
{
	context.set_time(time);
	dirty_time_=time;
}

void
Layer::set_time(IndependentContext context, Time time, const Point &pos)const
{
	context.set_time(time,pos);
	dirty_time_=time;
}

void
Layer::set_render_method(Context context, RenderMethod x)
{
	context.set_render_method(x);
}

Color
Layer::get_color(Context context, const Point &pos)const
{
	return context.get_color(pos);
}

CairoColor
Layer::get_cairocolor(Context context, const Point &pos)const
{
	// When the layer doesn't define its own get_cairocolor
	// then the normal get_cairo color will be used and 
	// a Color to CairoColor conversion will be done. 
	return CairoColor(get_color(context, pos));
}


synfig::Layer::Handle
Layer::hit_check(synfig::Context context, const synfig::Point &pos)const
{
	return context.hit_check(pos);
}

// Temporary function to render transformed layer for layers which yet not suppurt transformed rendering
#ifdef _DEBUG
bool
Layer::render_transformed(const Layer *layer, Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb, const char *file, int line)
{
	warning("%s:%d Resampling used while rendering - possible overhead (called from %s:%d)", __FILE__, __LINE__, file, line);
#else
bool
Layer::render_transformed(const Layer *layer, Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb, const char *, int)
{
#endif

	Transformation transformation(renddesc.get_transformation_matrix());

	if(cb && !cb->amount_complete(0,10000)) return false;

	SuperCallback stageone(cb,0,4500,10000);
	SuperCallback stagetwo(cb,4500,9000,10000);
	SuperCallback stagethree(cb,9000,9999,10000);

	surface->set_wh(renddesc.get_w(),renddesc.get_h());
	surface->clear();

	const Rect full_bounding_rect(layer->get_full_bounding_rect(context));
	Rect inner_bounds(
	    full_bounding_rect.get_min(),
	    full_bounding_rect.get_max()
	);
	inner_bounds &= transformation.back_transform_bounds(renddesc.get_rect());
	Rect outer_bounds(transformation.transform_bounds(inner_bounds));
	outer_bounds &= renddesc.get_rect();

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
	if (!inner_bounds.is_valid())
	{
		warning("%s:%d bounding box shrank while rendering?", __FILE__, __LINE__);
		return true;
	}

	Vector width_vector(
		transformation.transform(
			Vector(inner_bounds.maxx - inner_bounds.minx, 0.0), false ));
	Vector pixels_width_vector(
		width_vector[0]/renddesc.get_pw(),
		width_vector[1]/renddesc.get_ph() );
	int inner_width_pixels = (int)ceil(pixels_width_vector.mag());

	Vector ortho_axis_x(width_vector.norm());
	Vector ortho_axis_y(-ortho_axis_x.perp());

	Vector height_vector(
		transformation.transform(
			Vector(0.0, inner_bounds.maxy - inner_bounds.miny), false ));
	Vector ortho_height_vector(
		ortho_axis_y * (height_vector*ortho_axis_y) );
	Vector pixels_height_vector(
		ortho_height_vector[0]/renddesc.get_pw(),
		ortho_height_vector[1]/renddesc.get_ph() );
	int inner_height_pixels = (int)ceil(pixels_height_vector.mag());

	// make 8 pixels border for bicubic resampling
	float intermediate_pw = (inner_bounds.maxx-inner_bounds.minx)/(float)inner_width_pixels;
	float intermediate_ph = (inner_bounds.maxy-inner_bounds.miny)/(float)inner_height_pixels;
	inner_bounds.maxx += 8.f*intermediate_pw;
	inner_bounds.minx -= 8.f*intermediate_pw;
	inner_bounds.maxy += 8.f*intermediate_ph;
	inner_bounds.miny -= 8.f*intermediate_ph;
	inner_width_pixels += 16;
	inner_height_pixels += 16;

	RendDesc intermediate_desc(renddesc);
	intermediate_desc.clear_flags();
	intermediate_desc.set_transformation_matrix(Matrix());
	intermediate_desc.set_tl(Vector(inner_bounds.minx,inner_bounds.maxy));
	intermediate_desc.set_br(Vector(inner_bounds.maxx,inner_bounds.miny));
	intermediate_desc.set_flags(0);
	intermediate_desc.set_w(inner_width_pixels);
	intermediate_desc.set_h(inner_height_pixels);

	Surface intermediate_surface;
	if(!layer->accelerated_render(context,&intermediate_surface,quality,intermediate_desc,&stagetwo))
		return false;

	Rect pixels_outer_bounds(
		Vector((outer_bounds.minx-renddesc.get_tl()[0])/renddesc.get_pw(),
			   (outer_bounds.miny-renddesc.get_tl()[1])/renddesc.get_ph()),
		Vector((outer_bounds.maxx-renddesc.get_tl()[0])/renddesc.get_pw(),
			   (outer_bounds.maxy-renddesc.get_tl()[1])/renddesc.get_ph())
	);

	int left   = (int)floor(pixels_outer_bounds.minx);
	int top    = (int)floor(pixels_outer_bounds.miny);
	int right  = (int)ceil (pixels_outer_bounds.maxx);
	int bottom = (int)ceil (pixels_outer_bounds.maxy);

	int w = min(surface->get_w(), renddesc.get_w());
	int h = min(surface->get_h(), renddesc.get_h());

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right > w) right = w;
	if (bottom > h) bottom = h;

	int decx = right - left;
	if (top < bottom && left < right) {
		Vector initial_outer_pos(left*renddesc.get_pw(), top*renddesc.get_ph());
		initial_outer_pos += renddesc.get_tl();
		Vector initial_inner_pos = transformation.back_transform(initial_outer_pos);
		Vector initial_inner_surface_pos(initial_inner_pos - intermediate_desc.get_tl());
		initial_inner_surface_pos[0] /= intermediate_desc.get_pw();
		initial_inner_surface_pos[1] /= intermediate_desc.get_ph();

		Vector initial_outer_pos01((left+1)*renddesc.get_pw(), top*renddesc.get_ph());
		initial_outer_pos01 += renddesc.get_tl();
		Vector initial_inner_pos01 = transformation.back_transform(initial_outer_pos01);
		Vector initial_inner_surface_pos01(initial_inner_pos01 - intermediate_desc.get_tl());
		initial_inner_surface_pos01[0] /= intermediate_desc.get_pw();
		initial_inner_surface_pos01[1] /= intermediate_desc.get_ph();

		Vector initial_outer_pos10(left*renddesc.get_pw(), (top+1)*renddesc.get_ph());
		initial_outer_pos10 += renddesc.get_tl();
		Vector initial_inner_pos10 = transformation.back_transform(initial_outer_pos10);
		Vector initial_inner_surface_pos10(initial_inner_pos10 - intermediate_desc.get_tl());
		initial_inner_surface_pos10[0] /= intermediate_desc.get_pw();
		initial_inner_surface_pos10[1] /= intermediate_desc.get_ph();

		Vector dx(initial_inner_surface_pos01 - initial_inner_surface_pos);
		Vector dy(initial_inner_surface_pos10 - initial_inner_surface_pos);

		Vector row_inner_surface_pos(initial_inner_surface_pos);
		Vector inner_surface_pos;

		Surface::pen pen(surface->get_pen(left, top));
		for(int y = top; y < bottom; y++) {
			inner_surface_pos = row_inner_surface_pos;
			for(int x = left; x < right; x++) {
				pen.put_value( intermediate_surface.cubic_sample(inner_surface_pos[0], inner_surface_pos[1]) );
				pen.inc_x();
				inner_surface_pos += dx;
			}
			pen.dec_x(decx);
			pen.inc_y();
			row_inner_surface_pos += dy;
		}
	}

	if(cb && !cb->amount_complete(10000,10000)) return false;

	return true;
}

/* 	The default accelerated renderer
**	is anything but accelerated...
*/
bool
Layer::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	handle<Target_Scanline> target=surface_target(surface);
	if(!target)
	{
		if(cb)cb->error(_("Unable to create surface target"));
		return false;
	}
	RendDesc desc=renddesc;
	target->set_rend_desc(&desc);

	// When we render, we want to
	// make sure that we are rendered too...
	// Since the context iterator is for
	// the layer after us, we need to back up.
	// This could be considered a hack, as
	// it is a possibility that we are indeed
	// not the previous layer.
	--context;

	return render(context,target,desc,cb);
	//return render_threaded(context,target,desc,cb,2);
}


bool
Layer::accelerated_cairorender(Context context, cairo_t *cr, int /*quality*/, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	// When we render, we want to
	// make sure that we are rendered too...
	// Since the context iterator is for
	// the layer after us, we need to back up.
	// This could be considered a hack, as
	// it is a possibility that we are indeed
	// not the previous layer.
	--context;
	
	return cairorender(context,cr,renddesc,cb);
}


String
Layer::get_name()const
{
	return get_param("name__").get(String());
}

String
Layer::get_local_name()const
{
	return get_param("local_name__").get(String());
}


Layer::Vocab
Layer::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc(param_z_depth,"z_depth")
		.set_local_name(_("Z Depth"))
		.set_animation_only(true)
		.set_description(_("Modifies the position of the layer in the layer stack"))
	);

	return ret;
}

void
Layer::get_times_vfunc(Node::time_set &set) const
{
	DynamicParamList::const_iterator 	i = dynamic_param_list_.begin(),
										end = dynamic_param_list_.end();

	for(; i != end; ++i)
	{
		const Node::time_set &tset = i->second->get_times();
		set.insert(tset.begin(),tset.end());
	}
}


void
Layer::add_to_group(const String&x)
{
	if(x==group_)
		return;
	if(!group_.empty())
		remove_from_all_groups();
	group_=x;
	signal_added_to_group()(group_);
}

void
Layer::remove_from_group(const String&x)
{
	if(group_==x)
		remove_from_all_groups();
}

void
Layer::remove_from_all_groups()
{
	if(group_.empty())
		return;
	signal_removed_from_group()(group_);
	group_.clear();
}

String
Layer::get_group()const
{
	return group_;
}

const String
Layer::get_param_local_name(const String &param_name)const
{
	ParamVocab vocab = get_param_vocab();
	// loop to find the parameter in the parameter vocab - this gives us its local name
	for (ParamVocab::iterator iter = vocab.begin(); iter != vocab.end(); iter++)
		if (iter->get_name() == param_name)
			return iter->get_local_name();
	return String();
}

synfig::Layer::LooseHandle
synfig::Layer::get_parent_paste_canvas_layer()const
{
	synfig::Canvas::LooseHandle canvas=get_canvas();
	if(canvas->parent())
	{
		synfig::Canvas::LooseHandle parent_canvas=canvas->parent();
		Canvas::iterator iter;
		for(iter=parent_canvas->begin();iter!=parent_canvas->end();++iter)
		{
			Layer::LooseHandle layer=iter->get();
			if(dynamic_cast<Layer_PasteCanvas*>(layer.get()) != NULL)
			{
				Layer_PasteCanvas* paste_canvas(static_cast<Layer_PasteCanvas*>(layer.get()));
				Canvas::Handle sub_canvas=paste_canvas->get_sub_canvas();
				if(sub_canvas==canvas)
					return layer;
			}
		}
		synfig::warning("Layer's canvas has parent canvas but I can't find a proper Layer_PasteCanvas in it");
		return NULL;
	}
	return NULL;
}

String
Layer::get_string()const
{
	return String("Layer: ") + get_non_empty_description();

}

Real
Layer::get_parent_canvas_grow_value()const
{
	Canvas::Handle parent_canvas(get_canvas());
	Real ret;
	if(parent_canvas)
	{
		ret=parent_canvas->get_grow_value();
	}
	else
	{
		ret=0.0;
	}
	return ret;
}

void
Layer::fill_sound_processor(SoundProcessor & /* soundProcessor */) const
{ }
