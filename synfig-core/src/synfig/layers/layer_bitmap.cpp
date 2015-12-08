/* === S Y N F I G ========================================================= */
/*!	\file layer_bitmap.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/misc>

#include "layer_bitmap.h"

#include <synfig/time.h>
#include <synfig/string.h>
#include <synfig/vector.h>

#include <synfig/context.h>
#include <synfig/time.h>
#include <synfig/color.h>
#include <synfig/surface.h>
#include <synfig/renddesc.h>
#include <synfig/target.h>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/paramdesc.h>

#include <synfig/rendering/common/task/tasksurface.h>
#include <synfig/rendering/common/task/tasksurfaceempty.h>
#include <synfig/rendering/common/task/tasksurfaceresample.h>

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
	method			(SOFTWARE),
	param_tl				(Point(-0.5,0.5)),
	param_br				(Point(0.5,-0.5)),
	param_c				(int(1)),
	param_gamma_adjust	(Real(1.0)),
	surface			(128,128), // TODO: may be 1x1 or 0x0 will be better?
	trimmed			(false)
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
synfig::Layer_Bitmap::set_param(const String & param, const ValueBase & value)
{
	IMPORT_VALUE(param_tl);
	IMPORT_VALUE(param_br);
	IMPORT_VALUE(param_c);
	IMPORT_VALUE_PLUS(param_gamma_adjust,
		if(param=="gamma_adjust"&& value.get_type()==type_real)
		{
			param_gamma_adjust.set(Real(1.0/value.get(Real())));
			return true;
		}
		);

	return Layer_Composite::set_param(param,value);
}

ValueBase
synfig::Layer_Bitmap::get_param(const String & param)const
{
	EXPORT_VALUE(param_tl);
	EXPORT_VALUE(param_br);
	EXPORT_VALUE(param_c);
	if(param=="gamma_adjust")
	{
		ValueBase ret=param_gamma_adjust;
		ret.set(1.0/param_gamma_adjust.get(Real()));
		return ret;
	}

	if(param=="_width")
	{
		ValueBase ret1(type_integer);
		ret1=int(width);
		ValueBase ret2(type_integer);
		switch (method)
		{
				case SOFTWARE:
				ret2=int(surface.get_w());
				break;
				case CAIRO:
				default:
				ret2=int(csurface.get_w());
				break;
		}
		if (trimmed) return ret1;
		return ret2;
	}
	if(param=="_height")
	{
		ValueBase ret1(type_integer);
		ret1=int(height);
		ValueBase ret2(type_integer);
		switch (method)
		{
			case SOFTWARE:
				ret2=int(surface.get_h());
				break;
			case CAIRO:
			default:
				ret2=int(csurface.get_h());
				break;
		}
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
		.set_static(true)
	);

	ret.push_back(ParamDesc("gamma_adjust")
		.set_local_name(_("Gamma Adjustment"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_Bitmap::hit_check(synfig::Context context, const synfig::Point &pos)const
{
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
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

void
synfig::Layer_Bitmap::set_render_method(Context context, RenderMethod x)
{
	set_method(x);
	context.set_render_method(x);
}

inline
const Color&
synfig::Layer_Bitmap::filter(Color& x)const
{
	Real gamma_adjust(param_gamma_adjust.get(Real()));
	if(gamma_adjust!=1.0)
	{
		x.set_r(powf((float)x.get_r(),gamma_adjust));
		x.set_g(powf((float)x.get_g(),gamma_adjust));
		x.set_b(powf((float)x.get_b(),gamma_adjust));
	}
	return x;
}

inline
const CairoColor&
synfig::Layer_Bitmap::filter(CairoColor& x)const
{
	Real gamma_adjust(param_gamma_adjust.get(Real()));
	if(gamma_adjust!=1.0)
	{
		x.set_r(powf((float)(x.get_r()/CairoColor::range),gamma_adjust)*CairoColor::range);
		x.set_g(powf((float)(x.get_g()/CairoColor::range),gamma_adjust)*CairoColor::range);
		x.set_b(powf((float)(x.get_b()/CairoColor::range),gamma_adjust)*CairoColor::range);
	}
	return x;
}


Color
synfig::Layer_Bitmap::get_color(Context context, const Point &pos)const
{
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	int c(param_c.get(int()));

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
			Mutex::Lock lock(mutex);

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


CairoColor
synfig::Layer_Bitmap::get_cairocolor(Context context, const Point &pos)const
{
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	int c(param_c.get(int()));

	Point surface_pos;
	
	if(!get_amount())
		return context.get_cairocolor(pos);
	
	surface_pos=pos-tl;
	
	surface_pos[0]/=br[0]-tl[0];
	if(surface_pos[0]<=1.0 && surface_pos[0]>=0.0)
	{
		surface_pos[1]/=br[1]-tl[1];
		if(surface_pos[1]<=1.0 && surface_pos[1]>=0.0)
		{
			Mutex::Lock lock(mutex);

			if (trimmed)
			{
				surface_pos[0]*=width;
				surface_pos[1]*=height;
				
				if (surface_pos[0] > left+surface.get_w() || surface_pos[0] < left || surface_pos[1] > top+surface.get_h() || surface_pos[1] < top)
					return context.get_cairocolor(pos);
				
				surface_pos[0] -= left;
				surface_pos[1] -= top;
			}
			else
			{
				surface_pos[0]*=csurface.get_w();
				surface_pos[1]*=csurface.get_h();
			}
			
			CairoColor ret(CairoColor::alpha());
			
			switch(c)
			{
				case 6:	// Undefined
				case 5:	// Undefined
				case 4:	// Undefined
				case 3:	// Cubic
					ret=csurface.cubic_sample_cooked(surface_pos[0],surface_pos[1]);
					break;
					
				case 2:	// Cosine
					ret=csurface.cosine_sample_cooked(surface_pos[0],surface_pos[1]);
					break;
				case 1:	// Linear
					ret=csurface.linear_sample_cooked(surface_pos[0],surface_pos[1]);
					break;
				case 0:	// Nearest Neighbor
				default:
				{
					int x(min(csurface.get_w()-1,max(0,round_to_int(surface_pos[0]))));
					int y(min(csurface.get_h()-1,max(0,round_to_int(surface_pos[1]))));
					ret= csurface[y][x];
				}
					break;
			}
			ret=ret.demult_alpha();
			ret=filter(ret);
			
			if(get_amount()==1 && get_blend_method()==Color::BLEND_STRAIGHT)
				return ret;
			else
				return CairoColor::blend(ret,context.get_cairocolor(pos),get_amount(),get_blend_method());
		}
	}
	return context.get_cairocolor(pos);
}


bool
Layer_Bitmap::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Mutex::Lock lock(mutex);

	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	int c(param_c.get(int()));
	Real gamma_adjust(param_gamma_adjust.get(Real()));

	int interp=c;
	if(quality>=10)
		interp=0;
	else if(quality>=5 && interp>1)
		interp=1;

	// We can only handle NN and Linear at the moment
	//if(interp>1)
	//	return Layer_Composite::accelerated_render(context,surface,quality,renddesc,cb);

	//if we don't actually have a valid surface just skip us
	if(!this->surface.is_valid())
	{
		// Render what is behind us
		return context.accelerated_render(surface,quality,renddesc,cb);
	}

	SuperCallback subcb(cb,1,10000,10001+renddesc.get_h());

	if(	get_amount()==1 &&
		get_blend_method()==Color::BLEND_STRAIGHT &&
		!trimmed &&
		renddesc.get_tl()==tl &&
		renddesc.get_br()==br)
	{
		// Check for the trivial case
		if(this->surface.get_w()==renddesc.get_w() && this->surface.get_h()==renddesc.get_h() && gamma_adjust==1.0f)
		{
			if(cb && !cb->amount_complete(0,100)) return false;
			*surface=this->surface;
			if(cb && !cb->amount_complete(100,100)) return false;
			return true;
		}
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
	}
	else
	{
		// Render what is behind us
		if(!context.accelerated_render(surface,quality,renddesc,&subcb))
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

	int		inw = this->surface.get_w();
	int		inh = this->surface.get_h();

	int		outw = renddesc.get_w();
	int		outh = renddesc.get_h();

	float	inwf, inhf;
	Point	itl, ibr;

	if (trimmed)
	{
		inwf = (br[0] - tl[0])*this->surface.get_w()/width;
		inhf = (br[1] - tl[1])*this->surface.get_h()/height;
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
	Surface::alpha_pen pen(surface->get_pen(x_start,y_start));
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
					Color rc = this->surface.sample_rect_clip(inx,iny,inx+indx,iny+indy);
					pen.put_value(filter(rc));
				}
				pen.dec_x(x_end-x_start);
			}

			//Color c = (*surface)[0][0];
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
				Color c = filter(this->surface[yclamp][xclamp]);
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
				Color col(this->surface.linear_sample(inx,iny));
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
				Color col(this->surface.cosine_sample(inx,iny));
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
				Color col(this->surface.cubic_sample(inx,iny));
				pen.put_value(filter(col));
			}
			pen.dec_x(x_end-x_start);

		}
	}

	return true;
}

/////
/////

bool
Layer_Bitmap::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	Mutex::Lock lock(mutex);

	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	int c(param_c.get(int()));
	Real gamma_adjust(param_gamma_adjust.get(Real()));
	
	int interp=c;
	if(quality>=10)
		interp=0;
	else if(quality>=5 && interp>1)
		interp=1;
	
	//if we don't actually have a valid surface just skip us
	if(!csurface.is_mapped())
	{
		// Render what is behind us
		return context.accelerated_cairorender(cr,quality,renddesc,cb);
	}
	
	cairo_surface_t* cs=csurface.get_cairo_image_surface();
	
	if(cairo_surface_status(cs) || cairo_surface_get_type(cs)!=CAIRO_SURFACE_TYPE_IMAGE)
	{
		// Render what is behind us
		return context.accelerated_cairorender(cr,quality,renddesc,cb);
	}
	
	SuperCallback subcb(cb,1,10000,10001+renddesc.get_h());
	
	Point	obr	= renddesc.get_br();
	Point   otl = renddesc.get_tl();
	
	int		outw = renddesc.get_w();
	int		outh = renddesc.get_h();
	
	int		inw = cairo_image_surface_get_width(cs);
	int		inh = cairo_image_surface_get_height(cs);
	
	
	if(	get_amount()==1 && // our bitmap is full opaque
	   get_blend_method()==Color::BLEND_STRAIGHT && // and it doesn't draw the context
	   otl==tl &&
	   obr==br) // and the tl and br are the same ...
	{
		// Check for the trivial case: the Bitmap and the destiny surface have same dimensions and there is not gamma adjust
		if(inw==outw && inh==outh && gamma_adjust==1.0f)
		{
			if(cb && !cb->amount_complete(0,100)) return false;
			{
				cairo_save(cr);
				cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE); // set operator to ignore destiny
				cairo_set_source_surface(cr, cs, 0, 0); // set the source our cairosurface
				cairo_paint(cr); // paint on the destiny
				cairo_restore(cr);
			}
			if(cb && !cb->amount_complete(100,100)) return false;
			return true;
		}
	}
	else // It is not the trivial case
	{
		// Render what is behind us...
		if(!context.accelerated_cairorender(cr,quality,renddesc,&subcb))
			return false;
	}
	
	if(cb && !cb->amount_complete(10000,10001+renddesc.get_h())) return false;
	
	
	// Calculate the width and height in pixels of the bitmap in the output surface
	float wp=(br[0]-tl[0])/renddesc.get_pw();
	float hp=(br[1]-tl[1])/renddesc.get_ph();
	// So we need to scale the bitmap by wp/inw in horizontal and hp/inh in vertical.
	float scalex=wp/inw;
	float scaley=hp/inh;
	// Now let's calculate the displacement of the image in the output surface.
	Point disp=tl-otl;
	// Calculate the cairo interpolation to do by the interpolation parameter c
	cairo_filter_t filter;
	switch(c)
	{
		case 3:	// Cubic
			filter=CAIRO_FILTER_BEST;
			break;
		case 2:	// Cosine
			filter=CAIRO_FILTER_GOOD;
			break;
		case 1:	// Linear
			filter=CAIRO_FILTER_FAST;
			break;
		case 0:	// Nearest Neighbor
		default:
			filter=CAIRO_FILTER_NEAREST;
			break;
	}
	// TODO: filter the image with gamma_adjust!!
	cairo_save(cr);
	// Need to scale down to user coordinates before pass to cr
	cairo_translate(cr, renddesc.get_tl()[0], renddesc.get_tl()[1]);
	cairo_scale(cr, renddesc.get_pw(), renddesc.get_ph());
	// Apply the bitmap scale and tanslate
	cairo_translate(cr, disp[0]/renddesc.get_pw(), disp[1]/renddesc.get_ph());
	cairo_scale(cr, scalex, scaley);
	// set the surface, filter, and paint
	cairo_pattern_set_filter(cairo_get_source(cr), filter);
	cairo_set_source_surface(cr, cs, 0,0);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	// we don't need cs anymore
	cairo_surface_destroy(cs);
	cairo_restore(cr);
	
	return true;
}

/////


Rect
Layer_Bitmap::get_bounding_rect()const
{
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));

	return Rect(tl,br);
}


void 
Layer_Bitmap::set_cairo_surface(cairo_surface_t *cs)
{
	if(cs==NULL)
	{
		synfig::error("Layer_Bitmap received a NULL cairo_surface_t");
		return;
	}
	if(cairo_surface_status(cs))
	{
		synfig::error("Layer_Bitmap received a non valid cairo_surface_t");
		return;
	}
	csurface.set_cairo_surface(cs);
	csurface.map_cairo_image();
}

rendering::Task::Handle
Layer_Bitmap::build_composite_task_vfunc(ContextParams /* context_params */) const
{
	if ( !rendering_surface
	  || !rendering_surface->is_created() )
		return new rendering::TaskSurfaceEmpty();

	rendering::TaskSurface::Handle task_surface(new rendering::TaskSurface());
	task_surface->target_surface = rendering_surface;
	task_surface->init_target_rect(
		RectInt(0, 0, task_surface->target_surface->get_width(), task_surface->target_surface->get_height()),
		param_tl.get(Vector()),
		param_br.get(Vector()) );

	rendering::TaskSurfaceResample::Handle task_resample(new rendering::TaskSurfaceResample());
	task_resample->gamma = (Color::value_type)param_gamma_adjust.get(Real());
	task_resample->interpolation = (Color::Interpolation)param_c.get(int());
	task_resample->antialiasing = true;
	task_resample->sub_task() = task_surface;
	return task_resample;
}
