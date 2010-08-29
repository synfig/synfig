/* === S Y N F I G ========================================================= */
/*!	\file layer_bitmap.cpp
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "layer_bitmap.h"
#include "layer.h"
#include "time.h"
#include "string.h"
#include "vector.h"

#include "context.h"
#include "time.h"
#include "color.h"
#include "surface.h"
#include "renddesc.h"
#include "target.h"

#include "general.h"
#include "paramdesc.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::Layer_Bitmap::Layer_Bitmap():
    Layer_Composite	(1.0,Color::BLEND_COMPOSITE),
	tl				(-0.5,0.5),
	br				(0.5,-0.5),
	c				(1),
	surface			(128,128),
	trimmed			(false),
	gamma_adjust	(1.0)
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
	set_param_static("c", true);
}

bool
synfig::Layer_Bitmap::set_param(const String & param, ValueBase value)
{
	IMPORT(tl);
	IMPORT(br);
	IMPORT(c);
	if(param=="gamma_adjust"&& value.get_type()==ValueBase::TYPE_REAL)
	{
		set_param_static(param, value.get_static());
		gamma_adjust=1.0/value.get(Real());
		//gamma_adjust.set_gamma(1.0/value.get(Real()));
		return true;
	}

	return Layer_Composite::set_param(param,value);
}

ValueBase
synfig::Layer_Bitmap::get_param(const String & param)const
{
	EXPORT(tl);
	EXPORT(br);
	EXPORT(c);
	if(param=="gamma_adjust")
	{
		ValueBase ret(1.0/gamma_adjust);
		ret.set_static(get_param_static(param));
		return ret;
	}

	if(param=="_width")
	{
		ValueBase ret1(ValueBase::TYPE_INTEGER);
		ret1=int(width);
		ValueBase ret2(surface.get_w());
		ret1.set_static(get_param_static(param));
		ret2.set_static(get_param_static(param));
		if (trimmed) return ret1;
		return ret2;
	}
	if(param=="_height")
	{
		ValueBase ret1(ValueBase::TYPE_INTEGER);
		ret1=int(height);
		ValueBase ret2(surface.get_h());
		ret1.set_static(get_param_static(param));
		ret2.set_static(get_param_static(param));
		if (trimmed) return ret1;
		return ret2;
	}

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Layer_Bitmap::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("tl")
		.set_local_name(_("Top-Left"))
		.set_description(_("Upper left-hand Corner of image"))
	);

	ret.push_back(ParamDesc("br")
		.set_local_name(_("Bottom-Right"))
		.set_description(_("Lower right-hand Corner of image"))
	);

	ret.push_back(ParamDesc("c")
		.set_local_name(_("Interpolation"))
		.set_description(_("What type of interpolation to use"))
		.set_hint("enum")
		.add_enum_value(0,"nearest",_("Nearest Neighbor"))
		.add_enum_value(1,"linear",_("Linear"))
		.add_enum_value(2,"cosine",_("Cosine"))
		.add_enum_value(3,"cubic",_("Cubic"))
	);

	ret.push_back(ParamDesc("gamma_adjust")
		.set_local_name(_("Gamma Adjustment"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_Bitmap::hit_check(synfig::Context context, const synfig::Point &pos)const
{
	Point surface_pos;
	surface_pos=pos-tl;

	surface_pos[0]/=br[0]-tl[0];
	if(surface_pos[0]<=1.0 && surface_pos[0]>=0.0)
	{
		surface_pos[1]/=br[1]-tl[1];
		if(surface_pos[1]<=1.0 && surface_pos[1]>=0.0)
		{
			return const_cast<Layer_Bitmap*>(this);
		}
	}

	return context.hit_check(pos);
}

inline
const Color&
synfig::Layer_Bitmap::filter(Color& x)const
{
	if(gamma_adjust!=1.0)
	{
		x.set_r(powf((float)x.get_r(),gamma_adjust));
		x.set_g(powf((float)x.get_g(),gamma_adjust));
		x.set_b(powf((float)x.get_b(),gamma_adjust));
	}
	return x;
}

Color
synfig::Layer_Bitmap::get_color(Context context, const Point &pos)const
{
	Point surface_pos;

	if(!get_amount())
		return context.get_color(pos);

	surface_pos=pos-tl;

	surface_pos[0]/=br[0]-tl[0];
	if(surface_pos[0]<=1.0 && surface_pos[0]>=0.0)
	{
		surface_pos[1]/=br[1]-tl[1];
		if(surface_pos[1]<=1.0 && surface_pos[1]>=0.0)
		{
			if (trimmed)
			{
				surface_pos[0]*=width;
				surface_pos[1]*=height;

				if (surface_pos[0] > left+surface.get_w() || surface_pos[0] < left || surface_pos[1] > top+surface.get_h() || surface_pos[1] < top)
					return context.get_color(pos);

				surface_pos[0] -= left;
				surface_pos[1] -= top;
			}
			else
			{
				surface_pos[0]*=surface.get_w();
				surface_pos[1]*=surface.get_h();
			}

			Color ret(Color::alpha());

			switch(c)
			{
			case 6:	// Undefined
			case 5:	// Undefined
			case 4:	// Undefined
			case 3:	// Cubic
				ret=surface.cubic_sample(surface_pos[0],surface_pos[1]);
				break;

			case 2:	// Cosine
				ret=surface.cosine_sample(surface_pos[0],surface_pos[1]);
				break;
			case 1:	// Linear
				ret=surface.linear_sample(surface_pos[0],surface_pos[1]);
				break;
			case 0:	// Nearest Neighbor
			default:
				{
					int x(min(surface.get_w()-1,max(0,round_to_int(surface_pos[0]))));
					int y(min(surface.get_h()-1,max(0,round_to_int(surface_pos[1]))));
					ret= surface[y][x];
				}
			break;
			}

			ret=filter(ret);

			if(get_amount()==1 && get_blend_method()==Color::BLEND_STRAIGHT)
				return ret;
			else
				return Color::blend(ret,context.get_color(pos),get_amount(),get_blend_method());
		}
	}

	return context.get_color(pos);
}

bool
Layer_Bitmap::accelerated_render(Context context,Surface *out_surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	int interp=c;
	if(quality>=10)
		interp=0;
	else if(quality>=5 && interp>1)
		interp=1;

	// We can only handle NN and Linear at the moment
	//if(interp>1)
	//	return Layer_Composite::accelerated_render(context,out_surface,quality,renddesc,cb);

	//if we don't actually have a valid surface just skip us
	if(!surface.is_valid())
	{
		// Render what is behind us
		return context.accelerated_render(out_surface,quality,renddesc,cb);
	}

	SuperCallback subcb(cb,1,10000,10001+renddesc.get_h());

	if(	get_amount()==1 &&
		get_blend_method()==Color::BLEND_STRAIGHT &&
		!trimmed &&
		renddesc.get_tl()==tl &&
		renddesc.get_br()==br)
	{
		// Check for the trivial case
		if(surface.get_w()==renddesc.get_w() && surface.get_h()==renddesc.get_h() && gamma_adjust==1.0f)
		{
			if(cb && !cb->amount_complete(0,100)) return false;
			*out_surface=surface;
			if(cb && !cb->amount_complete(100,100)) return false;
			return true;
		}
		out_surface->set_wh(renddesc.get_w(),renddesc.get_h());
	}
	else
	{
		// Render what is behind us
		if(!context.accelerated_render(out_surface,quality,renddesc,&subcb))
			return false;
	}

	if(cb && !cb->amount_complete(10000,10001+renddesc.get_h())) return false;

	Point	obr	= renddesc.get_br(),
			otl = renddesc.get_tl();

	//Vector::value_type pw=renddesc.get_w()/(renddesc.get_br()[0]-renddesc.get_tl()[0]);
	//Vector::value_type ph=renddesc.get_h()/(renddesc.get_br()[1]-renddesc.get_tl()[1]);

	//A = representation of input (just tl,br) 	//just a scaling right now
	//B = representation of output (just tl,br)	//just a scaling right now
	//sa = scaling for input (0,1) -> (0,w/h)
	//sb = scaling for output (0,1) -> (0,w/h)

	float	outwf = obr[0] - otl[0];
	float	outhf = obr[1] - otl[1];

	int		inw = surface.get_w();
	int		inh = surface.get_h();

	int		outw = renddesc.get_w();
	int		outh = renddesc.get_h();

	float	inwf, inhf;
	Point	itl, ibr;

	if (trimmed)
	{
		inwf = (br[0] - tl[0])*surface.get_w()/width;
		inhf = (br[1] - tl[1])*surface.get_h()/height;
		itl = Point(tl[0] + (br[0]-tl[0])*left/width,
					tl[1] + (br[1]-tl[1])*top/height);
		ibr = Point(tl[0] + (br[0]-tl[0])*(left+inw)/width,
					tl[1] + (br[1]-tl[1])*(top+inh)/height);
	}
	else
	{
		inwf = br[0] - tl[0];
		inhf = br[1] - tl[1];
		itl = tl;
		ibr = br;
	}

	//need to get the input coords in output space, so we can clip

	//get the desired corners of the bitmap (in increasing order) in integers
	//floating point corners
	float x1f = (itl[0] - otl[0])*outw/outwf;
	float x2f = (ibr[0] - otl[0])*outw/outwf;
	float y1f = (itl[1] - otl[1])*outh/outhf;
	float y2f = (ibr[1] - otl[1])*outh/outhf;

	if(x1f > x2f) swap(x1f,x2f);
	if(y1f > y2f) swap(y1f,y2f);

	int x_start = max(0,(int)floor(x1f));	//probably floor
	int x_end 	= min(outw,(int)ceil(x2f));	//probably ceil
	int y_start = max(0,(int)floor(y1f));	//probably floor
	int y_end	= min(outh,(int)ceil(y2f));	//probably ceil

	//need to get the x,y,dx,dy values from output space to input, so we can do fast interpolation

	//get the starting position in input space... for interpolating

	// in int -> out float:
	// Sb(B^-1)A(Sa^-1) x
	float inx_start = (((x_start/*+0.5f*/)*outwf/outw + otl[0]) - itl[0])*inw/inwf; //may want to bias this (center of pixel)???
	float iny_start = (((y_start/*+0.5f*/)*outhf/outh + otl[1]) - itl[1])*inh/inhf; //may want to bias this (center of pixel)???

	//calculate the delta values in input space for one pixel movement in output space
	//same matrix but with a vector instead of a point...
	float indx = outwf*(inw)/((outw)*inwf);		//translations died
	float indy = outhf*(inh)/((outh)*inhf);		//translations died

	//perhaps use a DDA algorithm... if faster...
	//   will still want pixel fractions to be floating point since colors are

	//synfig::info("xstart:%d ystart:%d xend:%d yend:%d",x_start,y_start,x_end,y_end);

	//start drawing at the start of the bitmap (either origin or corner of input...)
	//and get other info
	Surface::alpha_pen pen(out_surface->get_pen(x_start,y_start));
	pen.set_alpha(get_amount());
	pen.set_blend_method(get_blend_method());

	//check if we should use the downscale filtering
	if(quality <= 7)
	{
		//the stride of the value should be inverted because we want to downsample
		//when the stride is small, not big
		//int multw = (int)ceil(indx);
		//int multh = (int)ceil(indy);

		if(indx > 1.7 || indy > 1.7)
		{
			/*synfig::info("Decided to downsample? ratios - (%f,%f) -> (%d,%d)",
						indx, indy, multw, multh);	*/

			//use sample rect here...

			float iny, inx;
			int x,y;

			//Point sample - truncate
			iny = iny_start;//+0.5f;
			for(y = y_start; y < y_end; ++y, pen.inc_y(), iny += indy)
			{
				inx = inx_start;//+0.5f;
				for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
				{
					Color rc = surface.sample_rect_clip(inx,iny,inx+indx,iny+indy);
					pen.put_value(filter(rc));
				}
				pen.dec_x(x_end-x_start);
			}

			//Color c = (*out_surface)[0][0];
			//synfig::info("ValueBase of first pixel = (%f,%f,%f,%f)",c.get_r(),c.get_g(),c.get_b(),c.get_a());

			return true;
		}
	}

	//perform normal interpolation
	if(interp==0)
	{
		//synfig::info("Decided to do nearest neighbor");
		float iny, inx;
		int x,y;

		//Point sample - truncate
		iny = iny_start;//+0.5f;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;//+0.5f;
			int yclamp = min(inh-1, max(0, round_to_int(iny)));
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				int xclamp = min(inw-1, max(0, round_to_int(inx)));
				Color c = filter(surface[yclamp][xclamp]);
				pen.put_value(c); //must get rid of the clip
			}
			pen.dec_x(x_end-x_start);
		}
	}
	else
	if(interp==1)
	{
		//bilinear filtering

		//float 	xmf,xpf,ymf,ypf;
		//int		xm,xp,ym,yp;
		float 	inx,iny;
		int		x,y;

		//can probably buffer for x values...

		//loop and based on inx,iny sample input image
		iny = iny_start;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				Color col(surface.linear_sample(inx,iny));
				pen.put_value(filter(col));
			}
			pen.dec_x(x_end-x_start);

		}
	}
	else
	if(interp==2)
	{
		//cosine filtering

		//float 	xmf,xpf,ymf,ypf;
		//int		xm,xp,ym,yp;
		float 	inx,iny;
		int		x,y;

		//can probably buffer for x values...

		//loop and based on inx,iny sample input image
		iny = iny_start;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				Color col(surface.cosine_sample(inx,iny));
				pen.put_value(filter(col));
			}
			pen.dec_x(x_end-x_start);

		}
	}
	else
	{
		//cubic filtering

		//float 	xmf,xpf,ymf,ypf;
		//int		xm,xp,ym,yp;
		float 	inx,iny;
		int		x,y;

		//can probably buffer for x values...

		//loop and based on inx,iny sample input image
		iny = iny_start;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				Color col(surface.cubic_sample(inx,iny));
				pen.put_value(filter(col));
			}
			pen.dec_x(x_end-x_start);

		}
	}

	return true;
}

Rect
Layer_Bitmap::get_bounding_rect()const
{
	return Rect(tl,br);
}
