/* ========================================================================
** Sinfg
** Template File
** $Id: surface.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#define SINFG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "canvas.h"
#include "surface.h"
#include "target_scanline.h"
#include "general.h"

#ifdef HAS_VIMAGE
#include <Accelerate/Accelerate.h>
#endif

#endif

using namespace sinfg;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

class target2surface : public sinfg::Target_Scanline
{
public:
	Surface *surface;
	bool	 sized;
public:
	target2surface(Surface *surface);
	virtual ~target2surface();

	virtual bool set_rend_desc(sinfg::RendDesc *newdesc);
	
	virtual bool start_frame(sinfg::ProgressCallback *cb);
	
	virtual void end_frame();

	virtual Color * start_scanline(int scanline);

	virtual bool end_scanline();
};

target2surface::target2surface(Surface *surface):surface(surface)
{
}

target2surface::~target2surface()
{
}

bool
target2surface::set_rend_desc(sinfg::RendDesc *newdesc)
{
	assert(newdesc);
	assert(surface);
	desc=*newdesc;
	return sinfg::Target_Scanline::set_rend_desc(newdesc);
}
	
bool
target2surface::start_frame(sinfg::ProgressCallback *cb) 
{ 
	if(surface->get_w() != desc.get_w() || surface->get_h() != desc.get_h())
	{
		surface->set_wh(desc.get_w(),desc.get_h());
	}
	return true; 
}
	
void
target2surface::end_frame()
{
	return;
}
	
Color *
target2surface::start_scanline(int scanline)
{
	return (*surface)[scanline];
}

bool
target2surface::end_scanline()
{
	return true;
}

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Scanline::Handle
sinfg::surface_target(Surface *surface)
{
	return Target_Scanline::Handle(new target2surface(surface));
}

void
sinfg::Surface::clear()
{
#ifdef HAS_VIMAGE
	fill(Color(0.5,0.5,0.5,0.0000001));
#else
	etl::surface<Color, ColorAccumulator, ColorPrep>::clear();
#endif
}

void
sinfg::Surface::blit_to(alpha_pen& pen, int x, int y, int w, int h)
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
		w = min(w,pen.end_x()-pen.x());
		h = min(h,pen.end_y()-pen.y());
		
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
		//top.rowBytes=get_w()*sizeof(Color); //! \fixme this should get the pitch!!
		top.rowBytes=get_pitch();

		bottom.data=static_cast<void*>(pen.x());
		bottom.height=h;
		bottom.width=w;
		//bottom.rowBytes=pen.get_width()*sizeof(Color); //! \fixme this should get the pitch!!
		bottom.rowBytes=pen.get_pitch(); //! \fixme this should get the pitch!!
				
		vImage_Error ret;
		ret=vImageAlphaBlend_ARGBFFFF(&top,&bottom,&dest,kvImageNoFlags);
		
		assert(ret!=kvImageNoError);
		
		return;
	}
#endif
	etl::surface<Color, ColorAccumulator, ColorPrep>::blit_to(pen,x,y,w,h);
}

