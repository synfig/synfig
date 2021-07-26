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

#include <synfig/layers/layer_composite.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#if HAVE_HARFBUZZ
#include <harfbuzz/hb.h>
#endif
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Layer_Freetype : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (synfig::String) text of the layer;
	synfig::ValueBase param_text;
	//!Parameter: (synfig::Color) color of the text;
	synfig::ValueBase param_color;
	//!Parameter: (synfig::String) font family used in the text
	synfig::ValueBase param_family;
	//!Parameter: (int) style used in the font
	synfig::ValueBase param_style;
	//!Parameter: (int) weight used in the font
	synfig::ValueBase param_weight;
	//!Parameter: (synfig::Real) horizontal spacing
	synfig::ValueBase param_compress;
	//!Parameter: (synfig::Real) vertical spacing
	synfig::ValueBase param_vcompress;
	//!Parameter: (synfig::Vector) size of the text
	synfig::ValueBase param_size;
	//!Parameter: (synfig::Vector) text orientation
	synfig::ValueBase param_orient;
	//!Parameter: (synfig::Point) text position
	synfig::ValueBase param_origin;
	//!Parameter: (synfig::String) font used in the text
	synfig::ValueBase param_font;
	//!Parameter: (bool)
	synfig::ValueBase param_use_kerning;
	//!Parameter: (bool)
	synfig::ValueBase param_grid_fit;
	//!Parameter: (bool) inverts the rendered text
	synfig::ValueBase param_invert;

	FT_Face face;
#if HAVE_HARFBUZZ
	hb_font_t *font;
#endif
	struct TextSpan
	{
		std::vector<uint32_t> codepoints;
#if HAVE_HARFBUZZ
		hb_script_t script;
#endif
	};

	typedef std::vector<TextSpan> TextLine;
	std::vector<TextLine> lines;

	bool font_path_from_canvas;

	bool old_version;
	std::atomic<bool> needs_sync {false};

	void sync();

	synfig::Color color_func(const synfig::Point &x, int quality=10, synfig::ColorReal supersample=0)const;

	mutable std::mutex mutex;
	mutable std::mutex sync_mtx;

public:
	Layer_Freetype();
	~Layer_Freetype() override;

	void on_canvas_set() override;
	bool set_param(const synfig::String & param, const synfig::ValueBase &value) override;
	synfig::ValueBase get_param(const synfig::String & param) const override;
	synfig::Color get_color(synfig::Context context, const synfig::Point &pos) const override;
	bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb) const override;

	Vocab get_param_vocab() const override;

	bool set_version(const synfig::String &ver) override { if (ver=="0.1") old_version=true; return true; }
	void reset_version() override {old_version=false;}

	synfig::Rect get_bounding_rect() const override;

private:
	void new_font(const synfig::String &family, int style=0, int weight=400);
	bool new_font_(const synfig::String &family, int style=0, int weight=400);
	bool new_face(const synfig::String &newfont);

	static std::vector<std::string> get_possible_font_directories(const std::string& canvas_path);

	void on_param_text_changed();

	static std::vector<TextLine> fetch_text_lines(const std::string& text);
};

/* === E N D =============================================================== */

#endif
