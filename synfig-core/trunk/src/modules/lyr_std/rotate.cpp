/*! ========================================================================
** Sinfg
** Template File
** $Id: rotate.cpp,v 1.2 2005/01/24 05:00:18 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "rotate.h"
#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <sinfg/transform.h>
#include <ETL/misc>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Rotate);
SINFG_LAYER_SET_NAME(Rotate,"rotate");
SINFG_LAYER_SET_LOCAL_NAME(Rotate,_("Rotate"));
SINFG_LAYER_SET_CATEGORY(Rotate,_("Transform"));
SINFG_LAYER_SET_VERSION(Rotate,"0.1");
SINFG_LAYER_SET_CVS_ID(Rotate,"$Id: rotate.cpp,v 1.2 2005/01/24 05:00:18 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Rotate::Rotate():
	origin	(0,0),
	amount	(Angle::deg(0)),
	sin_val	(0),
	cos_val	(1)
{
}

Rotate::~Rotate()
{
}

bool
Rotate::set_param(const String & param, const ValueBase &value)
{
	IMPORT(origin);
	
	if(param=="amount" && value.same_as(amount))
	{
		amount=value.get(amount);
		sin_val=Angle::sin(amount).get();
		cos_val=Angle::cos(amount).get();
		return true;
	}
	
	return false;
}

ValueBase
Rotate::get_param(const String &param)const
{
	EXPORT(origin);
	EXPORT(amount);
	
	EXPORT_NAME();
	EXPORT_VERSION();
		
	return ValueBase();	
}

Layer::Vocab
Rotate::get_param_vocab()const
{
	Layer::Vocab ret;
	
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Point where you want the origin to be"))
	);

	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
		.set_description(_("Amount of rotation"))
		.set_origin("origin")
	);
	
	return ret;
}

class Rotate_Trans : public Transform
{
	etl::handle<const Rotate> layer;
public:
	Rotate_Trans(const Rotate* x):Transform(x->get_guid()),layer(x) { }
	
	sinfg::Vector perform(const sinfg::Vector& x)const
	{
		Point pos(x-layer->origin);
		return Point(layer->cos_val*pos[0]-layer->sin_val*pos[1],layer->sin_val*pos[0]+layer->cos_val*pos[1])+layer->origin;
	}
	
	sinfg::Vector unperform(const sinfg::Vector& x)const
	{
		Point pos(x-layer->origin);
		return Point(layer->cos_val*pos[0]+layer->sin_val*pos[1],-layer->sin_val*pos[0]+layer->cos_val*pos[1])+layer->origin;
	}
};
etl::handle<Transform>
Rotate::get_transform()const
{
	return new Rotate_Trans(this);
}

sinfg::Layer::Handle
Rotate::hit_check(sinfg::Context context, const sinfg::Point &p)const
{
	Point pos(p-origin);
	Point newpos(cos_val*pos[0]+sin_val*pos[1],-sin_val*pos[0]+cos_val*pos[1]);
	newpos+=origin;
	return context.hit_check(newpos);
}

Color
Rotate::get_color(Context context, const Point &p)const
{
	Point pos(p-origin);
	Point newpos(cos_val*pos[0]+sin_val*pos[1],-sin_val*pos[0]+cos_val*pos[1]);
	newpos+=origin;
	return context.get_color(newpos);
}

bool
Rotate::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if(amount.dist(Angle::deg(0))==Angle::deg(0))
		return context.accelerated_render(surface,quality,renddesc,cb);
	if(amount.dist(Angle::deg(180))==Angle::deg(0))
	{
		RendDesc desc(renddesc);
		desc.clear_flags();
		Point tmp;
		tmp=renddesc.get_tl()-origin;
		desc.set_tl(Point(-tmp[0],-tmp[1])+origin);
		tmp=renddesc.get_br()-origin;
		desc.set_br(Point(-tmp[0],-tmp[1])+origin);
		return context.accelerated_render(surface,quality,desc,cb);
	}

	SuperCallback stageone(cb,0,9000,10000);
	SuperCallback stagetwo(cb,9000,10000,10000);
	
	if(cb && !cb->amount_complete(0,10000))
		return false;
	
	Point tl(renddesc.get_tl()-origin);
	Point br(renddesc.get_br()-origin);
	Point rot_tl(cos_val*tl[0]+sin_val*tl[1],-sin_val*tl[0]+cos_val*tl[1]);
	Point rot_br(cos_val*br[0]+sin_val*br[1],-sin_val*br[0]+cos_val*br[1]);	
	Point rot_tr(cos_val*br[0]+sin_val*tl[1],-sin_val*br[0]+cos_val*tl[1]);
	Point rot_bl(cos_val*tl[0]+sin_val*br[1],-sin_val*tl[0]+cos_val*br[1]);	
	rot_tl+=origin;
	rot_br+=origin;
	rot_tr+=origin;
	rot_bl+=origin;
	
	Point min_point(min(min(min(rot_tl[0],rot_br[0]),rot_tr[0]),rot_bl[0]),min(min(min(rot_tl[1],rot_br[1]),rot_tr[1]),rot_bl[1]));
	Point max_point(max(max(max(rot_tl[0],rot_br[0]),rot_tr[0]),rot_bl[0]),max(max(max(rot_tl[1],rot_br[1]),rot_tr[1]),rot_bl[1]));

	if(tl[0]>br[0])
	{
		tl[0]=max_point[0];
		br[0]=min_point[0];
	}
	else
	{
		br[0]=max_point[0];
		tl[0]=min_point[0];
	}
	if(tl[1]>br[1])
	{
		tl[1]=max_point[1];
		br[1]=min_point[1];
	}
	else
	{
		br[1]=max_point[1];
		tl[1]=min_point[1];
	}
	
	Real pw=(renddesc.get_w())/(renddesc.get_br()[0]-renddesc.get_tl()[0]);
	Real ph=(renddesc.get_h())/(renddesc.get_br()[1]-renddesc.get_tl()[1]);
	
	RendDesc desc(renddesc);
	desc.clear_flags();
	//desc.set_flags(RendDesc::PX_ASPECT);
	desc.set_tl(tl);
	desc.set_br(br);
	desc.set_wh(round_to_int(pw*(br[0]-tl[0])),round_to_int(ph*(br[1]-tl[1])));

	//sinfg::warning("given window: [%f,%f]-[%f,%f] %dx%d",renddesc.get_tl()[0],renddesc.get_tl()[1],renddesc.get_br()[0],renddesc.get_br()[1],renddesc.get_w(),renddesc.get_h());
	//sinfg::warning("surface to render: [%f,%f]-[%f,%f] %dx%d",desc.get_tl()[0],desc.get_tl()[1],desc.get_br()[0],desc.get_br()[1],desc.get_w(),desc.get_h());
		
	Surface source;
	source.set_wh(desc.get_w(),desc.get_h());

	if(!context.accelerated_render(&source,quality,desc,&stageone))
		return false;
	
	surface->set_wh(renddesc.get_w(),renddesc.get_h());

	Surface::pen pen(surface->begin());

	if(quality<=4)
	{
		// CUBIC
		int x,y;//,u,v,u2,v2;
		Point point,tmp;
		for(y=0,point[1]=renddesc.get_tl()[1];y<surface->get_h();y++,pen.inc_y(),pen.dec_x(x),point[1]+=1.0/ph)
		{
			for(x=0,point[0]=renddesc.get_tl()[0];x<surface->get_w();x++,pen.inc_x(),point[0]+=1.0/pw)
			{
				tmp=Point(cos_val*(point[0]-origin[0])+sin_val*(point[1]-origin[1]),-sin_val*(point[0]-origin[0])+cos_val*(point[1]-origin[1])) +origin;
				(*surface)[y][x]=source.cubic_sample((tmp[0]-tl[0])*pw,(tmp[1]-tl[1])*ph);
			}
			if(y&31==0 && cb)
			{
				if(!stagetwo.amount_complete(y,surface->get_h()))
					return false;
			}
		}
	}
	else
	if(quality<=6)
	{
		// INTERPOLATION_LINEAR
		int x,y;//,u,v,u2,v2;
		Point point,tmp;
		for(y=0,point[1]=renddesc.get_tl()[1];y<surface->get_h();y++,pen.inc_y(),pen.dec_x(x),point[1]+=1.0/ph)
		{
			for(x=0,point[0]=renddesc.get_tl()[0];x<surface->get_w();x++,pen.inc_x(),point[0]+=1.0/pw)
			{
				tmp=Point(cos_val*(point[0]-origin[0])+sin_val*(point[1]-origin[1]),-sin_val*(point[0]-origin[0])+cos_val*(point[1]-origin[1])) +origin;
				(*surface)[y][x]=source.linear_sample((tmp[0]-tl[0])*pw,(tmp[1]-tl[1])*ph);
			}
			if(y&31==0 && cb)
			{
				if(!stagetwo.amount_complete(y,surface->get_h()))
					return false;
			}
		}
	}
	else
	{
		// NEAREST_NEIGHBOR
		int x,y,u,v;
		Point point,tmp;
		for(y=0,point[1]=renddesc.get_tl()[1];y<surface->get_h();y++,pen.inc_y(),pen.dec_x(x),point[1]+=1.0/ph)
		{
			for(x=0,point[0]=renddesc.get_tl()[0];x<surface->get_w();x++,pen.inc_x(),point[0]+=1.0/pw)
			{
				tmp=Point(cos_val*(point[0]-origin[0])+sin_val*(point[1]-origin[1]),-sin_val*(point[0]-origin[0])+cos_val*(point[1]-origin[1])) +origin;
				u=int((tmp[0]-tl[0])*pw);
				v=int((tmp[1]-tl[1])*ph);
				if(u<0)
					u=0;
				if(v<0)
					v=0;
				if(u>=source.get_w())
					u=source.get_w()-1;
				if(v>=source.get_h())
					v=source.get_h()-1;
				//pen.set_value(source[v][u]);
				(*surface)[y][x]=source[v][u];
			}
			if(y&31==0 && cb)
			{
				if(!stagetwo.amount_complete(y,surface->get_h()))
					return false;
			}
		}
	}

	if(cb && !cb->amount_complete(10000,10000)) return false;

	return true;
}

Rect
Rotate::get_full_bounding_rect(Context context)const
{
	Rect under(context.get_full_bounding_rect());
	return get_transform()->perform(under);
}

