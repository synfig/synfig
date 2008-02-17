/* === S Y N F I G ========================================================= */
/*!	\file duckmatic.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif
#include <fstream>
#include <iostream>
#include <algorithm>

#include <ETL/hermite>

#include "duckmatic.h"
#include <synfigapp/value_desc.h>
#include <synfig/general.h>
#include <synfig/paramdesc.h>
#include <synfig/valuenode_timedswap.h>
#include <synfig/valuenode_animated.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_scale.h>
#include <synfig/valuenode_bline.h>

#include <synfig/curve_helper.h>

#include <sigc++/retype_return.h>
#include <sigc++/retype.h>
#include <sigc++/hide.h>
#include <sigc++/bind.h>

#include "canvasview.h"

#include "onemoment.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* 0.33333333333333333 makes for nice short tangent handles,
   1.0 makes them draw as their real length */
#define TANGENT_HANDLE_SCALE 0.33333333333333333

/* leave this alone or the bezier won't lie on top of the bline */
#define TANGENT_BEZIER_SCALE 0.33333333333333333

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Duckmatic::Duckmatic():
	type_mask(Duck::TYPE_ALL-Duck::TYPE_WIDTH),
	grid_snap(false),
	guide_snap(false),
	grid_size(1.0/4.0,1.0/4.0),
	show_persistent_strokes(true)
{
	axis_lock=false;
	drag_offset_=Point(0,0);
	clear_duck_dragger();
}

Duckmatic::~Duckmatic()
{
	clear_ducks();

	if (Duck::duck_count)
		synfig::error("%d ducks not yet deleted!", Duck::duck_count);

	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("Duckmatic::~Duckmatic(): Deleted");
}

void
Duckmatic::set_flip_x(bool flip_x)
{
	flip_x_ = flip_x;
}

void
Duckmatic::set_flip_y(bool flip_y)
{
	flip_y_ = flip_y;
}

void
Duckmatic::clear_ducks()
{
	duck_data_share_map.clear();
	duck_map.clear();

	//duck_list_.clear();
	bezier_list_.clear();
	stroke_list_.clear();

	if(show_persistent_strokes)
		stroke_list_=persistent_stroke_list_;
}

//! Returns \a true if the given duck is currently selected
bool
Duckmatic::duck_is_selected(const etl::handle<Duck> &duck)const
{
	return duck && selected_ducks.count(duck->get_guid());
}

void
Duckmatic::set_grid_size(const synfig::Vector &s)
{
	if(grid_size!=s)
	{
		grid_size=s;
		signal_grid_changed();
	}
}

void
Duckmatic::set_grid_snap(bool x)
{
	if(grid_snap!=x)
	{
		grid_snap=x;
		signal_grid_changed();
	}
}

void
Duckmatic::set_guide_snap(bool x)
{
	if(guide_snap!=x)
	{
		guide_snap=x;
		signal_grid_changed();
	}
}

Duckmatic::GuideList::iterator
Duckmatic::find_guide_x(synfig::Point pos, float radius)
{
	GuideList::iterator iter,best(guide_list_x_.end());
	float dist(radius);
	for(iter=guide_list_x_.begin();iter!=guide_list_x_.end();++iter)
	{
		float amount(abs(*iter-pos[0]));
		if(amount<dist)
		{
			dist=amount;
			best=iter;
		}
	}
	return best;
}

Duckmatic::GuideList::iterator
Duckmatic::find_guide_y(synfig::Point pos, float radius)
{
	GuideList::iterator iter,best(guide_list_y_.end());
	float dist(radius);
	for(iter=guide_list_y_.begin();iter!=guide_list_y_.end();++iter)
	{
		float amount(abs(*iter-pos[1]));
		if(amount<=dist)
		{
			dist=amount;
			best=iter;
		}
	}
	return best;
}

void
Duckmatic::clear_selected_ducks()
{
	selected_ducks.clear();
	signal_duck_selection_changed_();
}

etl::handle<Duckmatic::Duck>
Duckmatic::get_selected_duck()const
{
	if(selected_ducks.empty() || duck_map.empty())
		return 0;
	return duck_map.find(*selected_ducks.begin())->second;
}

void
Duckmatic::refresh_selected_ducks()
{
/*
	std::set<etl::handle<Duck> >::iterator iter;
	std::set<etl::handle<Duck> > new_set;
	if(duck_list().empty())
	{
		selected_duck_list.clear();
		signal_duck_selection_changed_();
		return;
	}

	for(iter=selected_duck_list.begin();iter!=selected_duck_list.end();++iter)
	{
		etl::handle<Duck> similar(find_similar_duck(*iter));
		if(similar)
		{
			new_set.insert(similar);
		}
	}
	selected_duck_list=new_set;
*/
	GUIDSet old_set(selected_ducks);
	GUIDSet::const_iterator iter;

	for(iter=old_set.begin();iter!=old_set.end();++iter)
	{
		if(duck_map.count(*iter)==0)
			selected_ducks.erase(*iter);
	}

	signal_duck_selection_changed_();
}

bool
Duckmatic::is_duck_group_selectable(const etl::handle<Duck>& x)const
{
	const Type type(get_type_mask());

	if (((x->get_type() && (!(type & x->get_type()))) ||
		 !x->get_editable()))
		return false;

	if(x->get_value_desc().parent_is_layer_param() && type & Duck::TYPE_POSITION)
	{
		Layer::Handle layer(x->get_value_desc().get_layer());
		String layer_name(layer->get_name());

		if (layer_name == "outline" || layer_name == "region" || layer_name == "plant" ||
			layer_name == "polygon" || layer_name == "curve_gradient")
			return false;

		if((layer_name=="PasteCanvas"|| layer_name=="paste_canvas") &&
		   !layer->get_param("children_lock").get(bool()))
			return false;
	}
	return true;
}

void
Duckmatic::select_all_ducks()
{
	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
		if(is_duck_group_selectable(iter->second))
			select_duck(iter->second);
		else
			unselect_duck(iter->second);
}

void
Duckmatic::toggle_select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br)
{
	Vector vmin, vmax;
	vmin[0]=std::min(tl[0],br[0]);
	vmin[1]=std::min(tl[1],br[1]);
	vmax[0]=std::max(tl[0],br[0]);
	vmax[1]=std::max(tl[1],br[1]);

	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
	{
		Point p(iter->second->get_trans_point());
		if(p[0]<=vmax[0] && p[0]>=vmin[0] && p[1]<=vmax[1] && p[1]>=vmin[1] &&
		   is_duck_group_selectable(iter->second))
			toggle_select_duck(iter->second);
	}
}

void
Duckmatic::select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br)
{
	Vector vmin, vmax;
	vmin[0]=std::min(tl[0],br[0]);
	vmin[1]=std::min(tl[1],br[1]);
	vmax[0]=std::max(tl[0],br[0]);
	vmax[1]=std::max(tl[1],br[1]);

//	Type type(get_type_mask());

	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
	{
		Point p(iter->second->get_trans_point());
		if(p[0]<=vmax[0] && p[0]>=vmin[0] && p[1]<=vmax[1] && p[1]>=vmin[1])
		{
			if(is_duck_group_selectable(iter->second))
				select_duck(iter->second);
		}
	}
}

int
Duckmatic::count_selected_ducks()const
{
	return selected_ducks.size();
}

void
Duckmatic::select_duck(const etl::handle<Duck> &duck)
{
	if(duck)
	{
		selected_ducks.insert(duck->get_guid());
		signal_duck_selection_changed_();
	}
}

DuckList
Duckmatic::get_selected_ducks()const
{
	DuckList ret;
	GUIDSet::const_iterator iter;
	const Type type(get_type_mask());

	for(iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		const DuckMap::const_iterator d_iter(duck_map.find(*iter));

		if(d_iter==duck_map.end())
			continue;

		if(( d_iter->second->get_type() && (!(type & d_iter->second->get_type())) ) )
			continue;

		ret.push_back(d_iter->second);
	}
	return ret;
}

DuckList
Duckmatic::get_duck_list()const
{
	DuckList ret;
	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
		ret.push_back(iter->second);
	return ret;
}

void
Duckmatic::unselect_duck(const etl::handle<Duck> &duck)
{
	if(duck && selected_ducks.count(duck->get_guid()))
	{
		selected_ducks.erase(duck->get_guid());
		signal_duck_selection_changed_();
	}
}

void
Duckmatic::toggle_select_duck(const etl::handle<Duck> &duck)
{
	if(duck_is_selected(duck))
		unselect_duck(duck);
	else
		select_duck(duck);
}

void
Duckmatic::translate_selected_ducks(const synfig::Vector& vector)
{
	if(duck_dragger_)
		duck_dragger_->duck_drag(this,vector);
}

void
Duckmatic::start_duck_drag(const synfig::Vector& offset)
{
	if(duck_dragger_)
		duck_dragger_->begin_duck_drag(this,offset);

	//drag_offset_=offset;
	drag_offset_=find_duck(offset)->get_trans_point();
}

bool
Duckmatic::end_duck_drag()
{
	if(duck_dragger_)
		return duck_dragger_->end_duck_drag(this);
	return false;
}

Point
Duckmatic::snap_point_to_grid(const synfig::Point& x, float radius)const
{
	Point ret(x);

	GuideList::const_iterator guide_x,guide_y;
	bool has_guide_x(false), has_guide_y(false);

	guide_x=find_guide_x(ret,radius);
	if(guide_x!=guide_list_x_.end())
		has_guide_x=true;

	guide_y=find_guide_y(ret,radius);
	if(guide_y!=guide_list_y_.end())
		has_guide_y=true;

	if(get_grid_snap())
	{
		Point snap(
			floor(ret[0]/get_grid_size()[0]+0.5)*get_grid_size()[0],
			floor(ret[1]/get_grid_size()[1]+0.5)*get_grid_size()[1]);

		if(abs(snap[0]-ret[0])<=radius && (!has_guide_x || abs(snap[0]-ret[0])<=abs(*guide_x-ret[0])))
			ret[0]=snap[0],has_guide_x=false;
		if(abs(snap[1]-ret[1])<=radius && (!has_guide_y || abs(snap[1]-ret[1])<=abs(*guide_y-ret[1])))
			ret[1]=snap[1],has_guide_y=false;
	}

	if(guide_snap)
	{
		if(has_guide_x)
			ret[0]=*guide_x;
		if(has_guide_y)
			ret[1]=*guide_y;
	}

	if(axis_lock)
	{
		ret-=drag_offset_;
		if(abs(ret[0])<abs(ret[1]))
			ret[0]=0;
		else
			ret[1]=0;
		ret+=drag_offset_;
	}

	return ret;
}

void
DuckDrag_Translate::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	last_translate_=Vector(0,0);
	{
		drag_offset_=duckmatic->find_duck(offset)->get_trans_point();

		snap=Vector(0,0);
	}

	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	positions.clear();
	for(iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		Point p((*iter)->get_trans_point());
		positions.push_back(p);
	}
}

bool
DuckDrag_Translate::end_duck_drag(Duckmatic* duckmatic)
{
	if(last_translate_.mag()>0.0001)
	{
		duckmatic->signal_edited_selected_ducks();
		return true;
	}
	else
	{
		duckmatic->signal_user_click_selected_ducks(0);
		return false;
	}
}

void
DuckDrag_Translate::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	synfig::Vector vect(duckmatic->snap_point_to_grid(vector)-drag_offset_);
	int i;

	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if((*iter)->get_type()!=Duck::TYPE_VERTEX &&
		   (*iter)->get_type()!=Duck::TYPE_POSITION)
			continue;
		(*iter)->set_trans_point(positions[i]+vect);
	}
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if((*iter)->get_type()==Duck::TYPE_VERTEX ||
		   (*iter)->get_type()==Duck::TYPE_POSITION)
			continue;
		(*iter)->set_trans_point(positions[i]+vect);
	}
	last_translate_=vect;
}

void
Duckmatic::signal_edited_selected_ducks()
{
	const DuckList ducks(get_selected_ducks());
	DuckList::const_iterator iter;

	synfig::GUIDSet old_set(selected_ducks);

	// If we have more than 20 things to move, then display
	// something to explain that it may take a moment
	smart_ptr<OneMoment> wait; if(ducks.size()>20)wait.spawn();

	// Go ahead and call everyone's signals
	for(iter=ducks.begin();iter!=ducks.end();++iter)
	{
		if ((*iter)->get_type() == Duck::TYPE_ANGLE)
		{
			if(!(*iter)->signal_edited_angle()((*iter)->get_rotations()))
			{
				selected_ducks=old_set;
				throw String("Bad edit");
			}
		}
		else if (App::restrict_radius_ducks &&
				 (*iter)->is_radius())
		{
			Point point((*iter)->get_point());
			bool changed = false;

			if (point[0] * (flip_x_ ? -1 : 1) < 0)
			{
				point[0] = 0;
				changed = true;
			}
			if (point[1] * (flip_y_ ? -1 : 1) < 0)
			{
				point[1] = 0;
				changed = true;
			}

			if (changed) (*iter)->set_point(point);

			if(!(*iter)->signal_edited()(point))
			{
				selected_ducks=old_set;
				throw String("Bad edit");
			}
		}
		else
		{
			if(!(*iter)->signal_edited()((*iter)->get_point()))
			{
				selected_ducks=old_set;
				throw String("Bad edit");
			}
		}
	}
	selected_ducks=old_set;
}

void
Duckmatic::signal_user_click_selected_ducks(int button)
{
	const DuckList ducks(get_selected_ducks());
	DuckList::const_iterator iter;

	for(iter=ducks.begin();iter!=ducks.end();++iter)
	{
		(*iter)->signal_user_click(button)();
	}
}

void
Duckmatic::add_duck(const etl::handle<Duck> &duck)
{
	//if(!duck_map.count(duck->get_guid()))
	{
		if(duck_data_share_map.count(duck->get_data_guid()))
		{
			duck->set_shared_point(duck_data_share_map[duck->get_data_guid()]);
		}
		else
		{
			etl::smart_ptr<synfig::Point> point(new Point(duck->get_point()));
			duck->set_shared_point(point);
			duck_data_share_map[duck->get_data_guid()]=point;
		}

		duck_map.insert(duck);
	}

	last_duck_guid=duck->get_guid();
}

void
Duckmatic::add_bezier(const etl::handle<Bezier> &bezier)
{
	bezier_list_.push_back(bezier);
}

void
Duckmatic::add_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color)
{
	assert(stroke_point_list);

	std::list<etl::handle<Stroke> >::iterator iter;

	for(iter=stroke_list_.begin();iter!=stroke_list_.end();++iter)
	{
		if((*iter)->stroke_data==stroke_point_list)
			return;
	}

	etl::handle<Stroke> stroke(new Stroke());

	stroke->stroke_data=stroke_point_list;
	stroke->color=color;

	stroke_list_.push_back(stroke);
}

void
Duckmatic::add_persistent_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color)
{
	add_stroke(stroke_point_list,color);
	persistent_stroke_list_.push_back(stroke_list_.back());
}

void
Duckmatic::clear_persistent_strokes()
{
	persistent_stroke_list_.clear();
}

void
Duckmatic::set_show_persistent_strokes(bool x)
{
	if(x!=show_persistent_strokes)
	{
		show_persistent_strokes=x;
		if(x)
			stroke_list_=persistent_stroke_list_;
		else
			stroke_list_.clear();
	}
}

void
Duckmatic::erase_duck(const etl::handle<Duck> &duck)
{
	duck_map.erase(duck->get_guid());
}

etl::handle<Duckmatic::Duck>
Duckmatic::find_similar_duck(etl::handle<Duck> duck)
{
	DuckMap::const_iterator iter(duck_map.find(duck->get_guid()));
	if(iter!=duck_map.end())
		return iter->second;
	return 0;

/*	std::list<handle<Duck> >::reverse_iterator iter;

	for(iter=duck_list_.rbegin();iter!=duck_list_.rend();++iter)
	{
		if(*iter!=duck && **iter==*duck)
		{
			//synfig::info("Found similar duck! (iter:%08x vs. duck:%08x)",iter->get(), duck.get());
			return *iter;
		}
	}
	return 0;
*/
}

etl::handle<Duckmatic::Duck>
Duckmatic::add_similar_duck(etl::handle<Duck> duck)
{
	etl::handle<Duck> similar(find_similar_duck(duck));
	if(!similar)
	{
		add_duck(duck);
		return duck;
	}
	return similar;
}

void
Duckmatic::erase_bezier(const etl::handle<Bezier> &bezier)
{
	std::list<handle<Bezier> >::iterator iter;

	for(iter=bezier_list_.begin();iter!=bezier_list_.end();++iter)
	{
		if(*iter==bezier)
		{
			bezier_list_.erase(iter);
			return;
		}
	}
	synfig::warning("Unable to find bezier to erase!");
}

etl::handle<Duckmatic::Duck>
Duckmatic::last_duck()const
{
	DuckMap::const_iterator iter(duck_map.find(last_duck_guid));
	if(iter!=duck_map.end())
		return iter->second;
	return 0;
}

etl::handle<Duckmatic::Bezier>
Duckmatic::last_bezier()const
{
	return bezier_list_.back();
}

etl::handle<Duckmatic::Duck>
Duckmatic::find_duck(synfig::Point point, synfig::Real radius, Duck::Type type)
{
	if(radius==0)radius=10000000;

	if(type==Duck::TYPE_DEFAULT)
		type=get_type_mask();

	Real closest(10000000);
	etl::handle<Duck> ret;

	DuckMap::const_iterator iter;

	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
	{
		const Duck::Handle& duck(iter->second);

		if(duck->get_ignore())
			continue;
		Real dist((duck->get_trans_point()-point).mag_squared());

		if(duck->get_type()&Duck::TYPE_VERTEX)
			dist*=1.0001;
		else if(duck->get_type()&Duck::TYPE_RADIUS)
			dist*=0.9999;

		if(dist<=closest && !( duck->get_type() && (!(type & duck->get_type())) ) )
		{
			{
				closest=dist;
				ret=duck;
			}
		}
	}

	if(radius==0 || closest<radius*radius)
		return ret;

	return 0;
}

etl::handle<Duckmatic::Bezier>
Duckmatic::find_bezier(synfig::Point point, synfig::Real radius,float* location)
{
	return find_bezier(point,radius,radius,location);
}

etl::handle<Duckmatic::Bezier>
Duckmatic::find_bezier(synfig::Point pos, synfig::Real scale, synfig::Real radius, float* location)
{
	if(radius==0)radius=10000000;
	Real closest(10000000);
	etl::handle<Bezier> ret;

	bezier<Point>	curve;

	Real 	d,step;
	float 	time = 0;
	float 	best_time = 0;

	for(std::list<handle<Bezier> >::const_iterator iter=bezier_list().begin();iter!=bezier_list().end();++iter)
	{
		curve[0] = (*iter)->p1->get_trans_point();
		curve[1] = (*iter)->c1->get_trans_point();
		curve[2] = (*iter)->c2->get_trans_point();
		curve[3] = (*iter)->p2->get_trans_point();
		curve.sync();

#if 0
		// I don't know why this doesn't work
		time=curve.find_closest(pos,6);
		d=((curve(time)-pos).mag_squared());

#else
		//set the step size based on the size of the picture
		d = (curve[1] - curve[0]).mag() + (curve[2]-curve[1]).mag()	+ (curve[3]-curve[2]).mag();

		step = d/(2*scale); //want to make the distance between lines happy

		step = max(step,0.01); //100 samples should be plenty
		step = min(step,0.1); //10 is minimum

		d = find_closest(curve,pos,step,&closest,&time);
#endif

		if(d < closest)
		{
			closest = d;
			ret = *iter;
			best_time=time;
		}
	}

	if(closest < radius*radius)
	{
		if(location)
			*location = best_time;	// We need to square-root this because we were dealing with squared distances

		return ret;
	}

	return 0;
}

bool
Duckmatic::save_sketch(const synfig::String& filename)const
{
	ChangeLocale change_locale(LC_NUMERIC, "C");
	std::ofstream file(filename.c_str());

	if(!file)return false;

	file<<"SKETCH"<<endl;

	std::list<etl::handle<Stroke> >::const_iterator iter;

	for(iter=persistent_stroke_list_.begin();iter!=persistent_stroke_list_.end();++iter)
	{
		file<<"C "
			<<(*iter)->color.get_r()<<' '
			<<(*iter)->color.get_g()<<' '
			<<(*iter)->color.get_b()
		<<endl;
		std::list<synfig::Point>::const_iterator viter;
		for(viter=(*iter)->stroke_data->begin();viter!=(*iter)->stroke_data->end();++viter)
		{
			file<<"V "
				<<(*viter)[0]<<' '
				<<(*viter)[1]
			<<endl;
		}
	}
	if(!file)return false;
	sketch_filename_=filename;
	signal_sketch_saved_();
	return true;
}

bool
Duckmatic::load_sketch(const synfig::String& filename)
{
	ChangeLocale change_locale(LC_NUMERIC, "C");
	std::ifstream file(filename.c_str());

	if(!file)
		return false;

	std::string line;
	getline(file,line);

	if(line!="SKETCH")
	{
		synfig::error("Not a sketch");
		return false;
	}

	etl::smart_ptr<std::list<synfig::Point> > stroke_data;

	while(file)
	{
		getline(file,line);

		if(line.empty())
			continue;

		switch(line[0])
		{
		case 'C':
		case 'c':
			{
				stroke_data.spawn();
				float r,g,b;
				if(!strscanf(line,"C %f %f %f",&r, &g, &b))
				{
					synfig::warning("Bad color line \"%s\"",line.c_str());
					r=0;g=0;b=0;
				}
				add_persistent_stroke(stroke_data, synfig::Color(r,g,b));
			}
			break;
		case 'V':
		case 'v':
			if(!stroke_data)
			{
				stroke_data.spawn();
				add_persistent_stroke(stroke_data, synfig::Color(0,0,0));
			}
			float x,y;
			if(!strscanf(line,"V %f %f",&x, &y))
				synfig::warning("Bad vertex \"%s\"",line.c_str());
			else
				stroke_data->push_back(synfig::Vector(x,y));
			break;
		default:
			synfig::warning("Unexpected sketch token '%c'",line[0]);
			break;
		}
	}

	sketch_filename_=filename;
	return true;
}

Duckmatic::Push::Push(Duckmatic *duckmatic_):
	duckmatic_(duckmatic_)
{
	duck_map=duckmatic_->duck_map;
	bezier_list_=duckmatic_->bezier_list_;
	duck_data_share_map=duckmatic_->duck_data_share_map;
	stroke_list_=duckmatic_->stroke_list_;
	duck_dragger_=duckmatic_->duck_dragger_;
	needs_restore=true;
}

Duckmatic::Push::~Push()
{
	if(needs_restore)
		restore();
}

void
Duckmatic::Push::restore()
{
	duckmatic_->duck_map=duck_map;
	duckmatic_->bezier_list_=bezier_list_;
	duckmatic_->duck_data_share_map=duck_data_share_map;
	duckmatic_->stroke_list_=stroke_list_;
	duckmatic_->duck_dragger_=duck_dragger_;
	needs_restore=false;
}

inline String guid_string(const synfigapp::ValueDesc& x)
{
	if(x.parent_is_layer_param())
		return strprintf("%s",x.get_layer()->get_guid().get_string().c_str())+x.get_param_name();
	//if(x.is_value_node())
		return strprintf("%s",x.get_value_node()->get_guid().get_string().c_str());
}

inline GUID calc_duck_guid(const synfigapp::ValueDesc& x,const synfig::TransformStack& transform_stack)
{
	GUID ret(0);

	if(x.parent_is_layer_param())
	{
		ret=x.get_layer()->get_guid()^GUID::hasher(x.get_param_name());
	}
	else
	{
		ret=x.get_value_node()->get_guid();
	}

	ret^=transform_stack.get_guid();
	return ret;
}

/*
Duck::Handle
Duckmatic::create_duck_from(const synfigapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const synfig::TransformStack& transform_stack, int modifier, synfig::ParamDesc *param_desc)
{
	GUID duck_guid(calc_duck_guid(value_desc,transform_stack)^GUID::hasher(modifier));
	etl::handle<Duck> duck=new Duck();

	return duck;
}
*/

bool
Duckmatic::add_to_ducks(const synfigapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const synfig::TransformStack& transform_stack, synfig::ParamDesc *param_desc, int multiple)
{
	ValueBase::Type type=value_desc.get_value_type();
#define REAL_COOKIE		reinterpret_cast<synfig::ParamDesc*>(28)
	switch(type)
	{
	case ValueBase::TYPE_REAL:

		if(!param_desc || param_desc==REAL_COOKIE || !param_desc->get_origin().empty())
		{
			etl::handle<Duck> duck=new Duck();
			duck->set_transform_stack(transform_stack);
			duck->set_radius(true);
			duck->set_type(Duck::TYPE_RADIUS);

			// put the duck on the right hand side of the center, whether the display is flipped or not
			duck->set_point(Point(value_desc.get_value(get_time()).get(Real()) * (flip_x_ ? -1 : 1), 0));
			duck->set_name(guid_string(value_desc));
			if(value_desc.is_value_node())
			{
				// If the ValueNode can be directly manipulated,
				// then set it as so.
				duck->set_editable(synfigapp::is_editable(value_desc.get_value_node()));
			}
			else
			{
				duck->set_editable(true);
			}

			if(param_desc && param_desc!=REAL_COOKIE)
			{
				if(!param_desc->get_origin().empty())
				{
					synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
					/*
					duck->set_origin(value_desc_origin.get_value(get_time()).get(synfig::Point()));
					*/
					add_to_ducks(value_desc_origin,canvas_view, transform_stack);
					duck->set_origin(last_duck());
				}
				duck->set_scalar(param_desc->get_scalar());
			}

			duck->signal_edited().clear(); // value_desc.get_value_type() == ValueBase::TYPE_REAL:
			duck->signal_edited().connect(
				sigc::bind(
					sigc::mem_fun(
						*canvas_view,
						&studio::CanvasView::on_duck_changed),
					value_desc));
			duck->set_value_desc(value_desc);

			duck->signal_user_click(2).connect(
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*canvas_view,
							&studio::CanvasView::popup_param_menu),
						0.0f),
					value_desc));

			duck->set_guid(calc_duck_guid(value_desc,transform_stack)^GUID::hasher(multiple));

			add_duck(duck);

			return true;
		}
		break;

	case ValueBase::TYPE_ANGLE:

		if(!param_desc || param_desc==REAL_COOKIE || !param_desc->get_origin().empty())
		{
			etl::handle<Duck> duck=new Duck();
			duck->set_type(Duck::TYPE_ANGLE);
			duck->set_transform_stack(transform_stack);
			synfig::Angle angle;

			angle=value_desc.get_value(get_time()).get(Angle());
			duck->set_point(Point(Angle::cos(angle).get(),Angle::sin(angle).get()));
			duck->set_name(guid_string(value_desc));
			if(value_desc.is_value_node())
			{
				ValueNode::Handle value_node=value_desc.get_value_node();
				//duck->set_name(strprintf("%x",value_node.get()));

				// If the ValueNode can be directly manipulated,
				// then set it as so.
				duck->set_editable(synfigapp::is_editable(value_desc.get_value_node()));
			}
			else
			{
				//angle=(value_desc.get_value().get(Angle()));
				//duck->set_point(Point(Angle::cos(angle).get(),Angle::sin(angle).get()));
				//duck->set_name(strprintf("%x",value_desc.get_layer().get())+value_desc.get_param_name());
				duck->set_editable(true);
			}

			if(param_desc && param_desc!=REAL_COOKIE)
			{
				if(!param_desc->get_origin().empty())
				{
					synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
					/*
					duck->set_origin(value_desc_origin.get_value(get_time()).get(synfig::Point()));
					*/
					add_to_ducks(value_desc_origin,canvas_view, transform_stack);
					duck->set_origin(last_duck());
				}
				duck->set_scalar(param_desc->get_scalar());
			}

			duck->signal_edited().clear(); // value_desc.get_value_type() == ValueBase::TYPE_ANGLE:
			duck->signal_edited_angle().clear();
			duck->signal_edited_angle().connect(
				sigc::bind(
					sigc::mem_fun(
						*canvas_view,
						&studio::CanvasView::on_duck_angle_changed),
					value_desc));
			duck->set_value_desc(value_desc);

			duck->signal_user_click(2).connect(
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*canvas_view,
							&studio::CanvasView::popup_param_menu),
						0.0f),
					value_desc));
			duck->set_guid(calc_duck_guid(value_desc,transform_stack)^GUID::hasher(multiple));

			add_duck(duck);

			return true;
		}
		break;

	case ValueBase::TYPE_VECTOR:
		{
			etl::handle<Duck> duck=new Duck();
			duck->set_transform_stack(transform_stack);

			duck->set_point(value_desc.get_value(get_time()).get(Point()));
			duck->set_name(guid_string(value_desc));
			if(value_desc.is_value_node())
			{
				//duck->set_name(strprintf("%x",value_desc.get_value_node().get()));

				// If the ValueNode can be directly manipulated,
				// then set it as so.
				duck->set_editable(synfigapp::is_editable(value_desc.get_value_node()));
			}
			else
			{
				//duck->set_point(value_desc.get_value().get(Point()));
				//duck->set_name(strprintf("%x",value_desc.get_layer().get())+value_desc.get_param_name());
				duck->set_editable(true);
			}

			// If we were passed a parameter description
			if(param_desc)
			{
				if(!param_desc->get_connect().empty())
				{
					synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_connect());
					Duck::Handle connect_duck;
					if(duck_map.find(calc_duck_guid(value_desc_origin,transform_stack)^GUID::hasher(0))!=duck_map.end())
					{
						connect_duck=duck_map[calc_duck_guid(value_desc_origin,transform_stack)^GUID::hasher(0)];
					}
					else
					{
						add_to_ducks(value_desc_origin,canvas_view, transform_stack);
						connect_duck=last_duck();
					}
					duck->set_connect_duck(connect_duck);
				}
				if(!param_desc->get_box().empty())
				{
					synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_box());
					add_to_ducks(value_desc_origin,canvas_view, transform_stack);
					duck->set_box_duck(last_duck());
				}

				// If we have an origin
				if(!param_desc->get_origin().empty())
				{
					synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
					/*
					duck->set_origin(value_desc_origin.get_value(get_time()).get(synfig::Point()));
					*/
					add_to_ducks(value_desc_origin,canvas_view, transform_stack);
					duck->set_origin(last_duck());
					duck->set_type(Duck::TYPE_VERTEX);
				}
				else
					duck->set_type(Duck::TYPE_POSITION);

				duck->set_scalar(param_desc->get_scalar());
			}
			else
				duck->set_type(Duck::TYPE_POSITION);

			duck->signal_edited().clear(); // value_desc.get_value_type() == ValueBase::TYPE_VECTOR:
			duck->signal_edited().connect(
				sigc::bind(
					sigc::mem_fun(
						*canvas_view,
						&studio::CanvasView::on_duck_changed),
					value_desc));
			duck->set_value_desc(value_desc);

			duck->signal_user_click(2).connect(
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*canvas_view,
							&studio::CanvasView::popup_param_menu),
						1.0f),
					value_desc));
			duck->set_guid(calc_duck_guid(value_desc,transform_stack)^GUID::hasher(multiple));
			add_duck(duck);

			return true;
		}
		break;
/*	case ValueBase::TYPE_SEGMENT:
		{
			etl::handle<Bezier> bezier(new Bezier());
			ValueNode_Composite::Handle value_node;

			if(value_desc.is_value_node() &&
				(value_node=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())))
			{
				if(!add_to_ducks(synfigapp::ValueDesc(value_node,0),canvas_view,transform_stack))
					return false;
				bezier->p1=last_duck();
				bezier->p1->set_type(Duck::TYPE_VERTEX);
				if(!add_to_ducks(synfigapp::ValueDesc(value_node,1),canvas_view,transform_stack))
					return false;
				bezier->c1=last_duck();
				bezier->c1->set_type(Duck::TYPE_TANGENT);
				bezier->c1->set_origin(bezier->p1);
				bezier->c1->set_scalar(TANGENT_BEZIER_SCALE);
				bezier->c1->set_tangent(true);

				if(!add_to_ducks(synfigapp::ValueDesc(value_node,2),canvas_view,transform_stack))
					return false;
				bezier->p2=last_duck();
				bezier->p2->set_type(Duck::TYPE_VERTEX);
				if(!add_to_ducks(synfigapp::ValueDesc(value_node,3),canvas_view,transform_stack))
					return false;
				bezier->c2=last_duck();
				bezier->c2->set_type(Duck::TYPE_TANGENT);
				bezier->c2->set_origin(bezier->p2);
				bezier->c2->set_scalar(-TANGENT_BEZIER_SCALE);
				bezier->c2->set_tangent(true);

				bezier->signal_user_click(2).connect(
					sigc::bind(
						sigc::mem_fun(
							*canvas_view,
							&studio::CanvasView::popup_param_menu_bezier),
						value_desc));

				add_bezier(bezier);
			}
			else if(value_desc.get_value().is_valid())
			{
				Segment segment=value_desc.get_value();
				etl::handle<Duck> duck_p,duck_c;
				synfig::String name;
				if(param_desc)
				{
					name=param_desc->get_local_name();
				}
				else
				{
					name=guid_string(value_desc);
				}

				duck_p=new class Duck(segment.p1);
				duck_p->set_name(name+".P1");
				duck_p->set_type(Duck::TYPE_VERTEX);
				add_duck(duck_p);

				duck_c=new class Duck(segment.t1);
				duck_c->set_name(name+".T1");
				duck_c->set_type(Duck::TYPE_TANGENT);
				add_duck(duck_c);
				duck_c->set_origin(duck_p);
				duck_c->set_scalar(TANGENT_HANDLE_SCALE);
				duck_c->set_tangent(true);

				bezier->p1=duck_p;
				bezier->c1=duck_c;

				duck_p=new class Duck(segment.p2);
				duck_p->set_name(name+".P2");
				duck_p->set_type(Duck::TYPE_VERTEX);
				add_duck(duck_p);

				duck_c=new class Duck(segment.t2);
				duck_c->set_type(Duck::TYPE_TANGENT);
				duck_c->set_name(name+".T2");
				add_duck(duck_c);
				duck_c->set_origin(duck_p);
				duck_c->set_scalar(-TANGENT_HANDLE_SCALE);
				duck_c->set_tangent(true);

				bezier->p2=duck_p;
				bezier->c2=duck_c;
				add_bezier(bezier);
			}

			return true;
		}
		break;
*/
	case ValueBase::TYPE_BLINEPOINT:
	{

		if(value_desc.is_value_node() &&
			ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueNode_Composite::Handle value_node;
			value_node=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());

			if(!add_to_ducks(synfigapp::ValueDesc(value_node,0),canvas_view,transform_stack))
				return false;
			etl::handle<Duck> vertex_duck(last_duck());
			vertex_duck->set_type(Duck::TYPE_VERTEX);
			if(!add_to_ducks(synfigapp::ValueDesc(value_node,4),canvas_view,transform_stack))
				return false;
			etl::handle<Duck> t1_duck(last_duck());

			t1_duck->set_origin(vertex_duck);
			t1_duck->set_scalar(-TANGENT_HANDLE_SCALE);
			t1_duck->set_tangent(true);

			etl::handle<Duck> t2_duck;

			// If the tangents are split
			if((*value_node->get_link("split"))(get_time()).get(bool()))
			{
				if(!add_to_ducks(synfigapp::ValueDesc(value_node,5),canvas_view,transform_stack))
					return false;
				t2_duck=last_duck();
				t2_duck->set_origin(vertex_duck);
				t2_duck->set_scalar(TANGENT_HANDLE_SCALE);
				t2_duck->set_tangent(true);
			}
			else
			{
				if(!add_to_ducks(synfigapp::ValueDesc(value_node,4),canvas_view,transform_stack))
					return false;
				t2_duck=last_duck();
				t2_duck->set_origin(vertex_duck);
				t2_duck->set_scalar(TANGENT_HANDLE_SCALE);
				t2_duck->set_tangent(true);
			}
			return true;
		}

	}
	break;
	case ValueBase::TYPE_LIST:
	{
		// Check for BLine
		if (value_desc.is_value_node() &&
			ValueNode_BLine::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueNode_BLine::Handle value_node;
			value_node=ValueNode_BLine::Handle::cast_dynamic(value_desc.get_value_node());

			int i,first=-1;

			etl::handle<Bezier> bezier;
			etl::handle<Duck> first_duck;
			etl::handle<Duck> duck, tduck;

			for (i = 0; i < value_node->link_count(); i++)
			{
				float amount(value_node->list[i].amount_at_time(get_time()));

				// skip vertices that aren't fully on
				if (amount < 0.9999f)
					continue;

				// remember the index of the first vertex we didn't skip
				if (first == -1)
					first = i;

				BLinePoint bline_point((*value_node->get_link(i))(get_time()));

				// try casting the vertex to Composite - this tells us whether it is composite or not
				ValueNode_Composite::Handle composite_vertex_value_node(
					ValueNode_Composite::Handle::cast_dynamic(value_node->get_link(i)));

				// add the vertex duck - it's a composite
				if(composite_vertex_value_node)
				{
					if (add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,0),canvas_view,transform_stack))
					{
						duck=last_duck();
						if(i==first)
							first_duck=duck;
						duck->set_type(Duck::TYPE_VERTEX);

						duck->signal_user_click(2).clear();
						duck->signal_user_click(2).connect(
							sigc::bind(
								sigc::bind(
									sigc::mem_fun(
										*canvas_view,
										&studio::CanvasView::popup_param_menu),
									1.0f),
								synfigapp::ValueDesc(value_node,i)));
						duck->set_value_desc(synfigapp::ValueDesc(value_node,i));

						if(param_desc)
						{
							if(!param_desc->get_origin().empty())
							{
								synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
								add_to_ducks(value_desc_origin,canvas_view, transform_stack);
								duck->set_origin(last_duck());
/*
								ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
								if(value.same_type_as(synfig::Point()))
									duck->set_origin(value.get(synfig::Point()));
*/
							}
						}
					}
					else
						return false;
				}
				// else it's not a composite
				else
				{
					duck=new Duck(bline_point.get_vertex());
					if(i==first)
						first_duck=duck;
					duck->set_transform_stack(transform_stack);
					duck->set_editable(false);
					//duck->set_name(strprintf("%x-vertex",value_node->get_link(i).get()));
					duck->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".v");

					duck->set_type(Duck::TYPE_VERTEX);
					if(param_desc)
					{
						if(!param_desc->get_origin().empty())
						{
								synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
								add_to_ducks(value_desc_origin,canvas_view, transform_stack);
								duck->set_origin(last_duck());
/*
								ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
								if(value.same_type_as(synfig::Point()))
									duck->set_origin(value.get(synfig::Point()));
*/
						}
					}
					duck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,i),transform_stack)^GUID::hasher(".v"));
					duck=add_similar_duck(duck);
//					add_duck(duck);
				}

				// Add the width duck if it is a parameter with a hint (ie. "width") or if it isn't a parameter
				if ((param_desc && !param_desc->get_hint().empty()) ||
					!param_desc)
				{
					etl::handle<Duck> width;
					if (add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,1),canvas_view,transform_stack,REAL_COOKIE))
					{
						width=last_duck();
						width->set_origin(duck);
						width->set_type(Duck::TYPE_WIDTH);
						width->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".w");

						// if the bline is a layer's parameter, scale the width duck by the layer's "width" parameter
						if (param_desc)
						{
							ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_hint()).get_value(get_time()));
							if(value.same_type_as(synfig::Real()))
								width->set_scalar(value.get(synfig::Real())*0.5f);
							// if it doesn't have a "width" parameter, scale by 0.5f instead
							else
								width->set_scalar(0.5f);
						}
						// otherwise just present the raw unscaled width
						else
							width->set_scalar(0.5f);
					}
					else
						synfig::error("Unable to add width duck!");
				}

				// each bezier uses t2 of one point and t1 of the next
				// the first time through this loop we won't have the t2 duck from the previous vertex
				// and so we don't make a bezier.  instead we skip on to t2 for this point
				if(bezier)
				{
					// Add the tangent1 duck
					if(composite_vertex_value_node)
					{
						if(!add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,4),canvas_view,transform_stack))
							return false;
						tduck=last_duck();
					}
					else
					{
						tduck=new Duck(bline_point.get_tangent1());
						tduck->set_transform_stack(transform_stack);
						tduck->set_editable(false);
						tduck->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".t1");
//						tduck->set_name(strprintf("%x-tangent1",value_node->get_link(i).get()));
						tduck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,i),transform_stack)^GUID::hasher(".t1"));
						tduck=add_similar_duck(tduck);
//						add_duck(duck);
					}

					tduck->set_origin(duck);
					tduck->set_scalar(-TANGENT_BEZIER_SCALE);
					tduck->set_tangent(true);

					bezier->p2=duck;
					bezier->c2=tduck;

					bezier->signal_user_click(2).connect(
						sigc::bind(
							sigc::mem_fun(
								*canvas_view,
								&studio::CanvasView::popup_param_menu_bezier),
							synfigapp::ValueDesc(value_node,i)));

					duck->signal_user_click(2).clear();
					duck->signal_user_click(2).connect(
						sigc::bind(
							sigc::bind(
								sigc::mem_fun(
									*canvas_view,
									&studio::CanvasView::popup_param_menu),
								1.0f),
							synfigapp::ValueDesc(value_node,i)));
					duck->set_value_desc(synfigapp::ValueDesc(value_node,i));

					add_bezier(bezier);
					bezier=0;
				}

				// don't start a new bezier for the last point in the line if we're not looped
				if(i+1>=value_node->link_count() && !value_node->get_loop())
					continue;

				bezier=new Bezier();

				// Add the tangent2 duck
				if(composite_vertex_value_node)
				{
					int i=bline_point.get_split_tangent_flag()?5:4;
					if(!add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,i),canvas_view,transform_stack,0,2))
						return false;
					tduck=last_duck();
				}
				else
				{
					if(bline_point.get_split_tangent_flag())
						tduck=new Duck(bline_point.get_tangent2());
					else
						tduck=new Duck(bline_point.get_tangent1());

					tduck->set_transform_stack(transform_stack);
					tduck->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".t2");
					tduck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,i),transform_stack)^GUID::hasher(".t2"));
					tduck->set_editable(false);
					tduck=add_similar_duck(tduck);
//					add_duck(duck);
					if(param_desc)
					{
								synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
								add_to_ducks(value_desc_origin,canvas_view, transform_stack);
								duck->set_origin(last_duck());
/*
								ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
								if(value.same_type_as(synfig::Point()))
									duck->set_origin(value.get(synfig::Point()));
*/
//						if(!param_desc->get_origin().empty())
//							duck->set_origin(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()).get(synfig::Point()));
					}
					duck->signal_user_click(2).clear();
					duck->signal_user_click(2).connect(
						sigc::bind(
							sigc::bind(
								sigc::mem_fun(
									*canvas_view,
									&studio::CanvasView::popup_param_menu),
								1.0f),
							synfigapp::ValueDesc(value_node,i)));
					duck->set_value_desc(synfigapp::ValueDesc(value_node,i));

				}

				tduck->set_origin(duck);
				tduck->set_scalar(TANGENT_BEZIER_SCALE);
				tduck->set_tangent(true);

				bezier->p1=duck;
				bezier->c1=tduck;
			}

			// Loop if necessary
			if(bezier && value_node->get_loop())
			{
				BLinePoint bline_point((*value_node->get_link(first))(get_time()));

				ValueNode_Composite::Handle composite_vertex_value_node(
					ValueNode_Composite::Handle::cast_dynamic(
						value_node->get_link(first)));

				// Add the vertex duck
				duck=first_duck;

				// Add the tangent1 duck
				if(composite_vertex_value_node)
				{
					if(!add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,4),canvas_view,transform_stack))
						return false;
					tduck=last_duck();
				}
				else
				{
					tduck=new Duck(bline_point.get_tangent1());
					tduck->set_transform_stack(transform_stack);
					tduck->set_editable(false);
					tduck->set_name(guid_string(synfigapp::ValueDesc(value_node,first))+".t1");
					//tduck->set_name(strprintf("%x-tangent1",value_node->get_link(first).get()));
					tduck=add_similar_duck(tduck);
					tduck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,first),transform_stack)^GUID::hasher(".t1"));
					//add_duck(duck);
				}

				tduck->set_origin(duck);
				tduck->set_scalar(-TANGENT_BEZIER_SCALE);
				tduck->set_tangent(true);

				bezier->p2=duck;
				bezier->c2=tduck;

				bezier->signal_user_click(2).connect(
					sigc::bind(
						sigc::mem_fun(
							*canvas_view,
							&studio::CanvasView::popup_param_menu_bezier),
						synfigapp::ValueDesc(value_node,first)));

				duck->signal_user_click(2).clear();
				duck->signal_user_click(2).connect(
					sigc::bind(
						sigc::bind(
							sigc::mem_fun(
								*canvas_view,
								&studio::CanvasView::popup_param_menu),
							1.0f),
						synfigapp::ValueDesc(value_node,first)));
				duck->set_value_desc(synfigapp::ValueDesc(value_node,first));

				add_bezier(bezier);
				bezier=0;
			}
			return true;
		}
		else // Check for DynamicList
		if(value_desc.is_value_node() &&
			ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueNode_DynamicList::Handle value_node;
			value_node=ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_value_node());
			int i;

			if(value_node->get_contained_type()==ValueBase::TYPE_VECTOR)
			{
				Bezier bezier;
				for(i=0;i<value_node->link_count();i++)
				{
					if(!value_node->list[i].status_at_time(get_time()))
						continue;
					if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
						return false;
					etl::handle<Duck> duck(last_duck());

					if(param_desc)
					{
								synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
								add_to_ducks(value_desc_origin,canvas_view, transform_stack);
								duck->set_origin(last_duck());
/*
								ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
								if(value.same_type_as(synfig::Point()))
									duck->set_origin(value.get(synfig::Point()));
*/
//						if(!param_desc->get_origin().empty())
//							last_duck()->set_origin(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()).get(synfig::Point()));
					}
					duck->set_type(Duck::TYPE_VERTEX);
					bezier.p1=bezier.p2;bezier.c1=bezier.c2;
					bezier.p2=bezier.c2=duck;

					if(i>0)
					{
						handle<Bezier> bezier_(new Bezier());
						bezier_->p1=bezier.p1;
						bezier_->c1=bezier.c1;
						bezier_->p2=bezier.p2;
						bezier_->c2=bezier.c2;
						add_bezier(bezier_);
						last_bezier()->signal_user_click(2).connect(
							sigc::bind(
								sigc::mem_fun(
									*canvas_view,
									&studio::CanvasView::popup_param_menu_bezier),
								synfigapp::ValueDesc(value_node,i)));
					}
				}
			}
			/*else if(value_node->get_contained_type()==ValueBase::TYPE_SEGMENT)
			{
				for(i=0;i<value_node->link_count();i++)
				{
					if(!value_node->list[i].status_at_time(get_time()))
						continue;
					if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
						return false;
				}
			}
			*/
			else
				return false;
		}
		else
		{
			// WRITEME
		}

		return true;
	}

	break;
	default:
		break;
	}
	return false;
}
