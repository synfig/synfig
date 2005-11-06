/* === S Y N F I G ========================================================= */
/*!	\file shade.cpp
**	\brief Template Header
**
**	$Id: shade.cpp,v 1.2 2005/01/24 03:08:17 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "shade.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>

#include <cstring>
#include <ETL/pen>
#include <ETL/misc>

#endif

using namespace synfig;
using namespace etl;
using namespace std;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_FASTGAUSSIAN	1
#define TYPE_CROSS			2
#define TYPE_GUASSIAN		3
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Layer_Shade);
SYNFIG_LAYER_SET_NAME(Layer_Shade,"shade");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Shade,_("Shade"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Shade,_("Stylize"));
SYNFIG_LAYER_SET_VERSION(Layer_Shade,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_Shade,"$Id: shade.cpp,v 1.2 2005/01/24 03:08:17 darco Exp $");

/* -- F U N C T I O N S ----------------------------------------------------- */

inline void clamp(synfig::Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Layer_Shade::Layer_Shade():
	Layer_Composite	(0.75,Color::BLEND_BEHIND),
	size(0.1,0.1),
	type(Blur::FASTGAUSSIAN),
	color(Color::black()),
	offset(0.2,-0.2),
	invert(false)
{
}

bool
Layer_Shade::set_param(const String &param, const ValueBase &value)
{
	IMPORT_PLUS(size,clamp(size));
	IMPORT(type);
	IMPORT(color);
	IMPORT(offset);
	IMPORT(invert);
	
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Shade::get_param(const String &param)const
{
	EXPORT(size);
	EXPORT(type);
	EXPORT(color);
	EXPORT(offset);
	EXPORT(invert);
	
	EXPORT_NAME();
	EXPORT_VERSION();
		
	return Layer_Composite::get_param(param);	
}

Color
Layer_Shade::get_color(Context context, const Point &pos)const
{
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==0.0)
		return context.get_color(pos);
	
	Color shade(color);

	if(!invert)
		shade.set_a(context.get_color(blurpos-offset).get_a());
	else
		shade.set_a(1.0f-context.get_color(blurpos-offset).get_a());

	return Color::blend(shade,context.get_color(pos),get_amount(),get_blend_method());
}

bool
Layer_Shade::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	int x,y;
	
	const int	w = renddesc.get_w(),
				h = renddesc.get_h();
	const Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();
	
	RendDesc	workdesc(renddesc);
	Surface		worksurface;
	etl::surface<float> blurred;
			
	//expand the working surface to accommodate the blur
	
	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (abs(size[0]*.5/pw) + 3),
		halfsizey = (int) (abs(size[1]*.5/ph) + 3);

	int offset_u(-round_to_int(offset[0]/pw)),offset_v(-round_to_int(offset[1]/ph));

	int offset_w(w+abs(offset_u)),offset_h(h+abs(offset_v));

	workdesc.set_subwindow(
		offset_u<0?offset_u:0,
		offset_v<0?offset_v:0,
		(offset_u>0?offset_u:0)+w,
		(offset_v>0?offset_v:0)+h
	);

	/*
	if(quality >=10)
	{
		halfsizex=1;
		halfsizey=1;
	}else
	*/
	if(quality == 9)
	{
		halfsizex/=4;
		halfsizey/=4;
	}
	
	//expand by 1/2 size in each direction on either side
	switch(type)
	{
		case Blur::DISC:
		case Blur::BOX:
		case Blur::CROSS:
		{
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),offset_w+2*max(1,halfsizex),offset_h+2*max(1,halfsizey));
			break;
		}
		case Blur::FASTGAUSSIAN:
		{
			if(quality < 4)
			{
				halfsizex*=2;
				halfsizey*=2;
			}
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),offset_w+2*max(1,halfsizex),offset_h+2*max(1,halfsizey));
			break;
		}
		case Blur::GAUSSIAN:
		{
		#define GAUSSIAN_ADJUSTMENT		(0.05)
			Real	pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
			Real 	ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);
			
			pw=pw*pw;
			ph=ph*ph;

			halfsizex = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);

			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			workdesc.set_subwindow( -halfsizex, -halfsizey, offset_w+2*halfsizex, offset_h+2*halfsizey );
						
			break;
		}
	}
#define SCALE_FACTOR	(64.0)
	if(/*quality>9 || */size[0]<=pw*SCALE_FACTOR)
	{
		SuperCallback stageone(cb,0,5000,10000);
		SuperCallback stagetwo(cb,5000,10000,10000);
		
		//callbacks depend on how long the blur takes
		if(size[0] || size[1])
		{
			if(type == Blur::DISC)
			{
				stageone = SuperCallback(cb,0,5000,10000);
				stagetwo = SuperCallback(cb,5000,10000,10000);	
			}
			else
			{
				stageone = SuperCallback(cb,0,9000,10000);
				stagetwo = SuperCallback(cb,9000,10000,10000);	
			}
		}
		else
		{
			stageone = SuperCallback(cb,0,9999,10000);
			stagetwo = SuperCallback(cb,9999,10000,10000);	
		}



		//render the background onto the expanded surface
		if(!context.accelerated_render(&worksurface,quality,workdesc,&stageone))
			return false;
	
		// Copy over the alpha
		blurred.set_wh(worksurface.get_w(),worksurface.get_h());
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
			{
				blurred[j][i]=worksurface[j][i].get_a();
			}
		
		//blur the image
		Blur(size,type,&stagetwo)(blurred,workdesc.get_br()-workdesc.get_tl(),blurred);
		
		//be sure the surface is of the correct size
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		
		int u = halfsizex-(offset_u<0?offset_u:0), v = halfsizey-(offset_v<0?offset_v:0);
		for(y=0;y<renddesc.get_h();y++,v++)
		{
			u = halfsizex-(offset_u<0?offset_u:0);
			for(x=0;x<renddesc.get_w();x++,u++)
			{
				Color a(color);
	
				if(!invert)
					a.set_a(blurred.linear_sample(offset_u+(float)u,offset_v+(float)v));
				else
					a.set_a(1.0f-blurred.linear_sample(offset_u+(float)u,offset_v+(float)v));
				
				if(a.get_a() || get_blend_method()==Color::BLEND_STRAIGHT)
				{
					(*surface)[y][x]=Color::blend(a,worksurface[v][u],get_amount(),get_blend_method());
				}
				else (*surface)[y][x] = worksurface[v][u];
			}
		}
	}
	else
	{
		
		SuperCallback stageone(cb,0,5000,10000);
		SuperCallback stagetwo(cb,5000,10000,10000);
		
		//callbacks depend on how long the blur takes
		if(size[0] || size[1])
		{
			if(type == Blur::DISC)
			{
				stageone = SuperCallback(cb,0,5000,10000);
				stagetwo = SuperCallback(cb,5000,10000,10000);	
			}
			else
			{
				stageone = SuperCallback(cb,0,9000,10000);
				stagetwo = SuperCallback(cb,9000,10000,10000);	
			}
		}
		else
		{
			stageone = SuperCallback(cb,0,9999,10000);
			stagetwo = SuperCallback(cb,9999,10000,10000);	
		}

		int fw(floor_to_int(abs(size[0]/(pw*SCALE_FACTOR)))+1);
		int fh(floor_to_int(abs(size[1]/(ph*SCALE_FACTOR)))+1);
		int tmpw(round_to_int((float)workdesc.get_w()/fw)),tmph(round_to_int((float)workdesc.get_h()/fh));
		
		workdesc.clear_flags();
		workdesc.set_wh(tmpw,tmph);
		//synfig::info("fw: %d, fh: %d",fw,fh);

		//render the blur fodder
		if(!context.accelerated_render(&worksurface,quality,workdesc,&stageone))
			return false;

		//render the background
		if(!context.accelerated_render(surface,quality,renddesc,&stageone))
			return false;

		// Copy over the alpha
		blurred.set_wh(worksurface.get_w(),worksurface.get_h());
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
				blurred[j][i]=worksurface[j][i].get_a();
		
		//blur the image
		Blur(size,type,&stagetwo)(blurred,workdesc.get_br()-workdesc.get_tl(),blurred);


		int u = halfsizex-(offset_u<0?offset_u:0), v = halfsizey-(offset_v<0?offset_v:0);
		for(y=0;y<renddesc.get_h();y++,v++)
		{
			u = halfsizex-(offset_u<0?offset_u:0);
			for(x=0;x<renddesc.get_w();x++,u++)
			{
				Color a(color);
	
				if(!invert)
					a.set_a(blurred.linear_sample(((float)offset_u+(float)u)/(float)fw,((float)offset_v+(float)v)/(float)fh));
				else
					a.set_a(1.0f-blurred.linear_sample(((float)offset_u+(float)u)/fw,((float)offset_v+(float)v)/(float)fh));
				
				if(a.get_a() || get_blend_method()==Color::BLEND_STRAIGHT)
				{
					(*surface)[y][x]=Color::blend(a,(*surface)[y][x],get_amount(),get_blend_method());
				}
			}
		}
	}
	
	
	if(cb && !cb->amount_complete(10000,10000))
	{
		//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
		
	return true;
}
	
Layer::Vocab
Layer_Shade::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
	);
	ret.push_back(ParamDesc("offset")
		.set_local_name(_("Offset"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of Shade"))
		.set_is_distance()
		.set_origin("offset")
	);
	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_description(_("Type of blur to use"))
		.set_hint("enum")
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);
	
	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert"))
	);
	
	return ret;
}

Rect
Layer_Shade::get_full_bounding_rect(Context context)const
{
	if(is_disabled())
		return context.get_full_bounding_rect();

	if(invert)
		return Rect::full_plane();

	Rect under(context.get_full_bounding_rect());

	if(Color::is_onto(get_blend_method()))
		return under;

	Rect bounds((under+offset).expand_x(size[0]).expand_y(size[1]));

	if(is_solid_color())
		return bounds;
		
	return bounds|under;
}
