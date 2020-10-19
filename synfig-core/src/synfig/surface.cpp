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
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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
#include "target_tile.h"
#include "general.h"
#include <synfig/localization.h>

#ifdef HAS_VIMAGE
#include <Accelerate/Accelerate.h>
#endif

#endif

using namespace synfig;
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




/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Tile::Handle
synfig::surface_target(Surface *surface, const String &engine)
	{ return Target_Tile::Handle(new target2surface(surface, engine)); }

Target_Scanline::Handle
synfig::surface_target_scanline(Surface *surface)
	{ return Target_Scanline::Handle(new target2surface_scanline(surface)); }



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
		w = std::min((long)w,(long)(pen.end_x()-pen.x()));
		h = std::min((long)h,(long)(pen.end_y()-pen.y()));

		//clip width against src width
		w = std::min(w,get_w()-x);
		h = std::min(h,get_h()-y);

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





