/* === S I N F G =========================================================== */
/*!	\file surface.h
**	\brief Surface and Pen Definitions
**
**	$Id: surface.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_SURFACE_H
#define __SINFG_SURFACE_H

/* === H E A D E R S ======================================================= */

#include "color.h"
#include "renddesc.h"
#include <ETL/pen>
#include <ETL/surface>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class Target;
class Target_Scanline;

class ColorPrep
{
public:
	ColorAccumulator cook(Color x)const
	{
		x.set_r(x.get_r()*x.get_a());
		x.set_g(x.get_g()*x.get_a());
		x.set_b(x.get_b()*x.get_a());
		return x;
	}
	Color uncook(ColorAccumulator x)const
	{
		if(!x.get_a())
			return Color::alpha();
		
		const float a(1.0f/x.get_a());
		
		x.set_r(x.get_r()*a);
		x.set_g(x.get_g()*a);
		x.set_b(x.get_b()*a);
		return x;
	}
};

/*!	\class Surface
**	\brief Bitmap Surface
**	\todo writeme
*/
class Surface : public etl::surface<Color, ColorAccumulator, ColorPrep>
{
public:
	typedef Color value_type;
	class alpha_pen;

	Surface() { }

	Surface(const size_type::value_type &w, const size_type::value_type &h):
		etl::surface<Color, ColorAccumulator,ColorPrep>(w,h) { }

	Surface(const size_type &s):
		etl::surface<Color, ColorAccumulator,ColorPrep>(s) { }

	template <typename _pen>
	Surface(const _pen &_begin, const _pen &_end):
		etl::surface<Color, ColorAccumulator,ColorPrep>(_begin,_end) { }

	template <class _pen> void blit_to(_pen &pen)
	{ return blit_to(pen,0,0, get_w(),get_h()); }

	template <class _pen> void
	blit_to(_pen& DEST_PEN,	int x, int y, int w, int h)
	{
		etl::surface<Color, ColorAccumulator, ColorPrep>::blit_to(DEST_PEN,x,y,w,h);
	}

	void clear();

	void blit_to(alpha_pen& DEST_PEN, int x, int y, int w, int h);
};	// END of class Surface

#ifndef DOXYGEN_SKIP

/*! \internal Used by Pen_Alpha */
struct _BlendFunc
{
	Color::BlendMethod blend_method;

	_BlendFunc(Color::BlendMethod b= Color::BLEND_COMPOSITE):blend_method(b) { }

	Color operator()(const Color &a,const Color &b,const Color::value_type &t)const
	{
		return Color::blend(b,a,t,blend_method);
	}
};	// END of class _BlendFunc

#endif

/*!	\class Surface::alpha_pen
**	\brief Alpha-Blending Pen
**
**	This pen works like a normal alpha pen, except that it supports
**	a variety of blending methods. Use set_blend_method() to select
**	which blending method you want to use.
**	The default blending method is Color::BLEND_COMPOSITE.
**	\see Color::BlendMethod
*/
class Surface::alpha_pen : public etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc >
{
public:
	alpha_pen() { }
	alpha_pen(const etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc > &x):
		etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc >(x)
	{ }
	
	alpha_pen(const etl::generic_pen<Color, ColorAccumulator>& pen, const Color::value_type &a = 1, const _BlendFunc &func = _BlendFunc()):
		etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc >(pen,a,func)
	{ }

	//! Sets the blend method to that described by \a method
	void set_blend_method(Color::BlendMethod method) { affine_func_.blend_method=method; }

	//! Returns the blend method being used for this pen
	Color::BlendMethod get_blend_method()const { return affine_func_.blend_method; }
};	// END of class Surface::alpha_pen

//! Creates a target that will render to \a surface
etl::handle<Target_Scanline> surface_target(Surface *surface);

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
