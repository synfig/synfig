/* === S Y N F I G ========================================================= */
/*!	\file surface.cpp
**	\brief Template File
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

#include "canvas.h"
#include "surface.h"
#include "target_scanline.h"
#include "target_cairo.h"
#include "target_tile.h"
#include "general.h"
#include <synfig/localization.h>

#ifdef HAS_VIMAGE
#include <Accelerate/Accelerate.h>
#endif

#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

class target2surface : public synfig::Target_Tile
{
public:
	String engine;
	Surface *surface;
	bool	 sized;
public:
	target2surface(Surface *surface, String engine):
		surface(surface), sized() { set_engine(engine); }

	virtual bool start_frame(synfig::ProgressCallback * /* cb */)
	{
		set_tile_w(desc.get_w());
		set_tile_h(desc.get_h());
		return true;
	}

	bool add_tile(const synfig::Surface &surface, int /* x */, int /* y */)
	{
		*this->surface = surface;
		return true;
	}

	void end_frame() { }
};

class target2surface_scanline : public synfig::Target_Scanline
{
public:
	Surface *surface;
	bool	 sized;

	target2surface_scanline(Surface *surface):
		surface(surface), sized() { }

	virtual bool set_rend_desc(synfig::RendDesc *newdesc)
	{
		assert(newdesc);
		assert(surface);
		return synfig::Target_Scanline::set_rend_desc(newdesc);
	}

	virtual bool start_frame(synfig::ProgressCallback * /* cb */)
	{
		if(surface->get_w() != desc.get_w() || surface->get_h() != desc.get_h())
			surface->set_wh(desc.get_w(),desc.get_h());
		return true;
	}

	virtual void end_frame()
		{ }

	virtual Color * start_scanline(int scanline)
		{ return (*surface)[scanline]; }

	virtual bool end_scanline()
		{ return true; }
};


class target2cairo_image: public synfig::Target_Cairo
{
public:
	cairo_surface_t** image;

	target2cairo_image(cairo_surface_t ** s):
		image(s) { }

	virtual bool set_rend_desc(synfig::RendDesc *newdesc)
	{
		assert(newdesc);
		return synfig::Target_Cairo::set_rend_desc(newdesc);
	}

	virtual bool obtain_surface(cairo_surface_t*& s)
	{
		int sw=cairo_image_surface_get_width(*image);
		int sh=cairo_image_surface_get_height(*image);
		int w=desc.get_w(), h=desc.get_h();

		if(sw!=w || sh!=h)
		{
			cairo_surface_destroy(*image);
			*image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
		}
		s=cairo_surface_reference(*image);
		return true;
	}
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Tile::Handle
synfig::surface_target(Surface *surface, const String &engine)
	{ return Target_Tile::Handle(new target2surface(surface, engine)); }

Target_Scanline::Handle
synfig::surface_target_scanline(Surface *surface)
	{ return Target_Scanline::Handle(new target2surface_scanline(surface)); }

Target_Cairo::Handle
synfig::cairo_image_target(cairo_surface_t **surface)
	{ return Target_Cairo::Handle(new target2cairo_image(surface)); }


void
synfig::Surface::clear()
{
#ifdef HAS_VIMAGE
	fill(Color(0.5,0.5,0.5,0.0000001));
#else
	etl::surface<Color, ColorAccumulator, ColorPrep>::clear();
#endif
}

void
synfig::Surface::blit_to(alpha_pen& pen, int x, int y, int w, int h)
{
	static const float epsilon(0.00001);
	const float alpha(pen.get_alpha());
	if(	pen.get_blend_method()==Color::BLEND_STRAIGHT && fabs(alpha-1.0f)<epsilon )
	{
		if(x>=get_w() || y>=get_w())
			return;

		//clip source origin
		if(x<0)
		{
			w+=x;	//decrease
			x=0;
		}

		if(y<0)
		{
			h+=y;	//decrease
			y=0;
		}

		//clip width against dest width
		w = min((long)w,(long)(pen.end_x()-pen.x()));
		h = min((long)h,(long)(pen.end_y()-pen.y()));

		//clip width against src width
		w = min(w,get_w()-x);
		h = min(h,get_h()-y);

		if(w<=0 || h<=0)
			return;

		for(int i=0;i<h;i++)
		{
			char* src(static_cast<char*>(static_cast<void*>(operator[](y)+x))+i*get_w()*sizeof(Color));
			char* dest(static_cast<char*>(static_cast<void*>(pen.x()))+i*pen.get_width()*sizeof(Color));
			memcpy(dest,src,w*sizeof(Color));
		}
		return;
	}

#ifdef HAS_VIMAGE
	if(	pen.get_blend_method()==Color::BLEND_COMPOSITE && fabs(alpha-1.0f)<epsilon )
	{
		if(x>=get_w() || y>=get_w())
			return;

		//clip source origin
		if(x<0)
		{
			//u-=x;	//increase
			w+=x;	//decrease
			x=0;
		}

		if(y<0)
		{
			//v-=y;	//increase
			h+=y;	//decrease
			y=0;
		}

		//clip width against dest width
		w = min(w,pen.end_x()-pen.x());
		h = min(h,pen.end_y()-pen.y());

		//clip width against src width
		w = min(w,get_w()-x);
		h = min(h,get_h()-y);

		if(w<=0 || h<=0)
			return;



		vImage_Buffer top,bottom;
		vImage_Buffer& dest(bottom);

		top.data=static_cast<void*>(operator[](y)+x);
		top.height=h;
		top.width=w;
		//top.rowBytes=get_w()*sizeof(Color); //! \todo this should get the pitch!!
		top.rowBytes=get_pitch();

		bottom.data=static_cast<void*>(pen.x());
		bottom.height=h;
		bottom.width=w;
		//bottom.rowBytes=pen.get_width()*sizeof(Color); //! \todo this should get the pitch!!
		bottom.rowBytes=pen.get_pitch(); //! \todo this should get the pitch!!

		vImage_Error ret;
		ret=vImageAlphaBlend_ARGBFFFF(&top,&bottom,&dest,kvImageNoFlags);

		assert(ret!=kvImageNoError);

		return;
	}
#endif
	etl::surface<Color, ColorAccumulator, ColorPrep>::blit_to(pen,x,y,w,h);
}

void
synfig::CairoSurface::blit_to(alpha_pen& pen, int x, int y, int w, int h)
{
	static const float epsilon(0.00001);
	const float alpha(pen.get_alpha());
	if(	pen.get_blend_method()==Color::BLEND_STRAIGHT && fabs(alpha-1.0f)<epsilon )
	{
		if(x>=get_w() || y>=get_w())
			return;
		
		//clip source origin
		if(x<0)
		{
			w+=x;	//decrease
			x=0;
		}
		
		if(y<0)
		{
			h+=y;	//decrease
			y=0;
		}
		
		//clip width against dest width
		w = min((long)w,(long)(pen.end_x()-pen.x()));
		h = min((long)h,(long)(pen.end_y()-pen.y()));
		
		//clip width against src width
		w = min(w,get_w()-x);
		h = min(h,get_h()-y);
		
		if(w<=0 || h<=0)
			return;
		
		for(int i=0;i<h;i++)
		{
			char* src(static_cast<char*>(static_cast<void*>(operator[](y)+x))+i*get_w()*sizeof(CairoColor));
			char* dest(static_cast<char*>(static_cast<void*>(pen.x()))+i*pen.get_width()*sizeof(CairoColor));
			memcpy(dest,src,w*sizeof(CairoColor));
		}
		return;
	}
	else
		etl::surface<CairoColor, CairoColorAccumulator, CairoColorPrep>::blit_to(pen,x,y,w,h);
}

void
CairoSurface::set_wh(int w, int h, int /*pitch*/)
{
	// If cs_ has been set then don't resize it.
	// Only the target which created the specific cairo surface can do that. 
	if(cs_!= NULL)
	{
		synfig::warning("Cannot resize a Cairo Surface directly. Use its Target_Cairo instance");
		return;
	}
	// decrease reference counter in case we have a valid cs_image_
	if(cs_image_!=NULL)
		cairo_surface_destroy(cs_image_);
	// create a new cairo_surface image type
	cs_image_=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	// check whether the new cairo surface is sane
	if(!cairo_surface_status(cs_image_))
	{
		int stride(cairo_image_surface_get_stride(cs_image_));
		unsigned char* data(cairo_image_surface_get_data(cs_image_));
		// pass the information to the etl::surface to be used directly.
		set_wh(w, h, data, stride);
	}
	else
		synfig::warning("CairoSurface::set_wh failed");
}

void 
CairoSurface::set_cairo_surface(cairo_surface_t *cs)
{
	if(cs==NULL)
	{
		if(is_mapped())
			unmap_cairo_image();
		cairo_surface_destroy(cs_);
		cs_=NULL;
		return;
	}
	if(cairo_surface_status(cs))
	{
		synfig::error("CairoSurface received a non valid cairo_surface_t");
		if(is_mapped())
			unmap_cairo_image();
		cairo_surface_destroy(cs_);
		cs_=NULL;
		return;
	}
	else
	{
		if(is_mapped())
			unmap_cairo_image();
		cairo_surface_destroy(cs_);
		cs_=cairo_surface_reference(cs);
	}
}

cairo_surface_t*
CairoSurface::get_cairo_surface()const
{
	if(cs_==NULL)
		return NULL;
	else
		return cairo_surface_reference(cs_);
}

cairo_surface_t*
CairoSurface::get_cairo_image_surface()const
{
	if(cs_image_==NULL)
		return NULL;
	else
		return cairo_surface_reference(cs_image_);
}


bool
CairoSurface::map_cairo_image()
{
	if(!cs_ || cs_image_)
	{
		synfig::error("Attempting to map a NULL surface or a surface already mapped)");
		return false;
	}
	if(cairo_surface_get_type(cs_) != CAIRO_SURFACE_TYPE_IMAGE)
	{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
		cs_image_=cairo_surface_map_to_image (cs_, NULL);
#else
		assert(false); // Old versions of cairo are not supported
#endif
	}
	else
	{
		cs_image_=cairo_surface_reference(cs_);
	}
	if(cs_image_!=NULL)
	{
		cairo_surface_flush(cs_image_);
		cairo_format_t t=cairo_image_surface_get_format(cs_image_);
		if(t==CAIRO_FORMAT_ARGB32 || t==CAIRO_FORMAT_RGB24)
		{
			unsigned char* data(cairo_image_surface_get_data(cs_image_));
			int w(cairo_image_surface_get_width(cs_image_));
			int h(cairo_image_surface_get_height(cs_image_));
			int stride(cairo_image_surface_get_stride(cs_image_));
			set_wh(w, h, data, stride);
			return true;
		}
		return false;	
	}
	return false;
}

void
CairoSurface::unmap_cairo_image()
{
	if(!cs_ || !cs_image_)
	{
		synfig::error("Attempting to unmap a NULL surface or a surface not mapped)");
		return;
	}
	cairo_surface_mark_dirty(cs_image_);
	if(cairo_surface_get_type(cs_) != CAIRO_SURFACE_TYPE_IMAGE)
	{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
		// this will destroy cs_image_
		cairo_surface_unmap_image(cs_, cs_image_);
		cs_image_=NULL;
#else
		assert(false); // Old versions of cairo are not supported
#endif
	}
	else
	{
		cairo_surface_destroy(cs_image_);
		cs_image_ = NULL;
	}
}


bool
CairoSurface::is_mapped()const
{
	return (cs_image_!=NULL);
}


