/* === S Y N F I G ========================================================= */
/*!	\file surface.h
**	\brief Surface and Pen Definitions
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_SURFACE_H
#define __SYNFIG_SURFACE_H

/* === H E A D E R S ======================================================= */

#include "color.h"
#include "renddesc.h"
#include <ETL/pen>
#include <ETL/surface>
#include <ETL/handle>

#include "cairo.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Target;
class Target_Scanline;
class Target_Cairo;
class Target_Tile;

class ColorPrep
{
public:
	static ColorAccumulator cook_static(Color x)
	{
		x.set_r(x.get_r()*x.get_a());
		x.set_g(x.get_g()*x.get_a());
		x.set_b(x.get_b()*x.get_a());
		return x;
	}
	static Color uncook_static(ColorAccumulator x)
	{
		if(!x.get_a())
			return Color::alpha();

		const float a(1.0f/x.get_a());

		x.set_r(x.get_r()*a);
		x.set_g(x.get_g()*a);
		x.set_b(x.get_b()*a);
		return x;
	}

	ColorAccumulator cook(Color x)const
		{ return cook_static(x); }
	Color uncook(ColorAccumulator x)const
		{ return uncook_static(x); }
};

class CairoColorPrep
{
public:
	CairoColor cook(CairoColor x)const
	{
		return x.premult_alpha();
	}
	CairoColor uncook(CairoColor x)const
	{
		return x.demult_alpha();
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


/*!	\class CairoSurface
 **	\brief Generic Cairo backed surface. It allows to create a image surface
 ** equivalent to the current backend for custom modifications purposes.
 **	\todo writeme
 */
class CairoSurface : public etl::surface<CairoColor, CairoColorAccumulator, CairoColorPrep>
{
	// This is the Cairo surface pointer
	// It is NULL if the not initialized
	cairo_surface_t *cs_;
	// This pointer is used when map and unmap the cairo_surface to a cairo_image_surface
	// see map_cairo_surface() unmap_cairo_surface();
	cairo_surface_t *cs_image_;
	
public:
	typedef CairoColor value_type;
	class alpha_pen;
	
	CairoSurface():cs_(NULL), cs_image_(NULL) {  }
	CairoSurface(cairo_surface_t *cs):cs_(NULL), cs_image_(NULL) { set_cairo_surface(cs); }
	~CairoSurface() { 
	if(cs_!= NULL) cairo_surface_destroy(cs_);
	if(cs_image_!=NULL) cairo_surface_destroy(cs_image_); }
	

	// If cs_ is set then the set_wh does nothing
	// If cs_ is not set then set_wh creates a cairo_surface_image on cs_image_
	// of size wxh
	void set_wh(int w, int h, int pitch=0);
	// Use whits version of set_wh to directly give to the etl::surface the 
	// pointer to data, the width, height and pitch (stride) between rows 
	void set_wh(int w, int h, unsigned char* data, int pitch)
	{ etl::surface<CairoColor, CairoColorAccumulator, CairoColorPrep>::set_wh(w, h, data, pitch); }
	// specialization of etl::surface::blit_to that considers the possibility of
	// don't blend colors when blend method is straight and alpha is 1.0
	void blit_to(alpha_pen& DEST_PEN, int x, int y, int w, int h);
	
	// Use this function to reference one given generic cairo_surface 
	// by this surface class.
	// When the CairoSurface instance is destructed the reference counter of the
	// cairo_surface_t should be decreased.
	// It is also possible to detach the cairo surface passing NULL as argument.
	// If the cairo surface is mapped at the time of call this function, then
	// it is unmaped first.
	void set_cairo_surface(cairo_surface_t *cs);
	// Returns an increased reference pointer of the surface. The receiver is responsible
	// of destroy the surface once referenced.
	cairo_surface_t* get_cairo_surface()const;
	// Returns an increased reference pointer of the image surface. The receiver is responsible
	// of destroy the surface once referenced.
	cairo_surface_t* get_cairo_image_surface()const;
	// Maps cs_ to cs_image_ and extract the *data to etl::surface::data for further modifications
	// It will flush any remaining painting operation to the cs_
	// returns true on success or false if something failed
	bool map_cairo_image();
	// Unmap the cs_image_ to cs_ after external modification has been done via *data
	// It will mark cs_ as dirty
	void unmap_cairo_image();
	// Returns true if the cairo_surface_t* cs_ is mapped on cs_image_
	bool is_mapped()const;
	
};	// END of class Surface


#ifndef DOXYGEN_SKIP

/*! \internal Used by Pen_Alpha */
template <class C, typename A=Color::value_type>
struct _BlendFunc
{
	Color::BlendMethod blend_method;

	_BlendFunc(typename Color::BlendMethod b= Color::BLEND_COMPOSITE):blend_method(b) { }

	C operator()(const C &a,const C &b,const A &t)const
	{
		return C::blend(b,a,t,blend_method);
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
class Surface::alpha_pen : public etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc<Color> >
{
public:
	alpha_pen() { }
	alpha_pen(const etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc<Color> > &x):
		etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc<Color> >(x)
	{ }

	alpha_pen(const etl::generic_pen<Color, ColorAccumulator>& pen, const Color::value_type &a = 1, const _BlendFunc<Color> &func = _BlendFunc<Color>()):
		etl::alpha_pen< etl::generic_pen<Color, ColorAccumulator>, Color::value_type, _BlendFunc<Color> >(pen,a,func)
	{ }

	//! Sets the blend method to that described by \a method
	void set_blend_method(Color::BlendMethod method) { affine_func_.blend_method=method; }

	//! Returns the blend method being used for this pen
	Color::BlendMethod get_blend_method()const { return affine_func_.blend_method; }
};	// END of class Surface::alpha_pen



/*!	\class CairoSurface::alpha_pen
 **	\brief Alpha-Blending Pen
 **
 **	This pen works like a normal alpha pen, except that it supports
 **	a variety of blending methods. Use set_blend_method() to select
 **	which blending method you want to use.
 **	The default blending method is Color::BLEND_COMPOSITE.
 **	\see Color::BlendMethod
 */
class CairoSurface::alpha_pen : public etl::alpha_pen< etl::generic_pen<CairoColor, CairoColorAccumulator>, float, _BlendFunc<CairoColor> >
{
public:
	alpha_pen() { }
	alpha_pen(const etl::alpha_pen< etl::generic_pen<CairoColor, CairoColorAccumulator>, float, _BlendFunc<CairoColor> > &x):
	etl::alpha_pen< etl::generic_pen<CairoColor, CairoColorAccumulator>, float, _BlendFunc<CairoColor> >(x)
	{ }
	
	alpha_pen(const etl::generic_pen<CairoColor, CairoColorAccumulator>& pen, const float &a = 1, const _BlendFunc<CairoColor> &func = _BlendFunc<CairoColor>()):
	etl::alpha_pen< etl::generic_pen<CairoColor, CairoColorAccumulator>, float, _BlendFunc<CairoColor> >(pen,a,func)
	{ }
	
	//! Sets the blend method to that described by \a method
	void set_blend_method(Color::BlendMethod method) { affine_func_.blend_method=method; }
	
	//! Returns the blend method being used for this pen
	Color::BlendMethod get_blend_method()const { return affine_func_.blend_method; }
};	// END of class CairoSurface::alpha_pen



//! Creates a target that will render to \a surface by specified \a renderer
etl::handle<Target_Tile> surface_target(Surface *surface, const String &renderer = String());
//! Creates a target that will render to \a surface by specified \a renderer
etl::handle<Target_Scanline> surface_target_scanline(Surface *surface);
//!Creates a target that will render to a cairo_surface_t image surface
etl::handle<Target_Cairo> cairo_image_target(cairo_surface_t** surface);

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
