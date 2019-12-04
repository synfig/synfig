/* === S Y N F I G ========================================================= */
/*!	\file lyr_freetype.h
**	\brief Header file for implementation of the "Text" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_LYR_FREETYPE_H
#define __SYNFIG_LYR_FREETYPE_H

/* === H E A D E R S ======================================================= */

//#ifdef __APPLE__
//#define USE_MAC_FT_FUNCS	(1)
//#endif

#include <synfig/layers/layer_composite.h>
#include <synfig/vector.h>
#include <synfig/color.h>
#include <synfig/string.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <vector>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/canvas.h>


#include <ETL/misc>

#ifdef USE_MAC_FT_FUNCS
	#include <CoreServices/CoreServices.h>
	#include FT_MAC_H
#endif

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;

struct Glyph
{
	FT_Glyph glyph;
	FT_Vector pos;
	//int width;
};
struct TextLine
{
	int width;
	std::vector<Glyph> glyph_table;

	TextLine():width(0) { }
	void clear_and_free();

	int actual_height()const
	{
		int height(0);

		std::vector<Glyph>::const_iterator iter;
		for(iter=glyph_table.begin();iter!=glyph_table.end();++iter)
		{
			FT_BBox   glyph_bbox;

			//FT_Glyph_Get_CBox( glyphs[n], ft_glyph_bbox_pixels, &glyph_bbox );
			FT_Glyph_Get_CBox( iter->glyph, ft_glyph_bbox_subpixels, &glyph_bbox );

			if(glyph_bbox.yMax>height)
				height=glyph_bbox.yMax;
		}
		return height;
	}
};


class Layer_Freetype : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (synfig::String) text of the layer;
	ValueBase param_text;
	//!Parameter: (synfig::Color) color of the text;
	ValueBase param_color;
	//!Parameter: (synfig::String) font family used in the text
	ValueBase param_family;
	//!Parameter: (int) style used in the font
	ValueBase param_style;
	//!Parameter: (int) weight used in the font
	ValueBase param_weight;
	//!Parameter: (synfig::Real) horizontal spacing
	ValueBase param_compress;
	//!Parameter: (synfig::Real) vertical spacing
	ValueBase param_vcompress;
	//!Parameter: (synfig::Vector) size of the text
	ValueBase param_size;
	//!Parameter: (synfig::Vector) text orientation
	ValueBase param_orient;
	//!Parameter: (synfig::Point) text position
	ValueBase param_origin;
	//!Parameter: (synfig::String) font used in the text
	ValueBase param_font;
	//!Parameter: (bool)
	ValueBase param_use_kerning;
	//!Parameter: (bool)
	ValueBase param_grid_fit;
	//!Parameter: (bool) inverts the rendered text
	ValueBase param_invert;

	FT_Face face;

	bool old_version;
	bool needs_sync_;

	void sync();

	synfig::Color color_func(const synfig::Point &x, int quality=10, synfig::ColorReal supersample=0)const;

	mutable std::mutex mutex;

public:
	Layer_Freetype();
	virtual ~Layer_Freetype();

	virtual void on_canvas_set();
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Color get_color(Context context, const synfig::Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context,cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

	virtual bool set_version(const String &ver){if(ver=="0.1")old_version=true;return true;}
	virtual void reset_version(){old_version=false;}

	virtual synfig::Rect get_bounding_rect()const;

private:
	void new_font(const synfig::String &family, int style=0, int weight=400);
	bool new_font_(const synfig::String &family, int style=0, int weight=400);
	bool new_face(const synfig::String &newfont);
};

extern FT_Library ft_library;

/* === E N D =============================================================== */

#endif
