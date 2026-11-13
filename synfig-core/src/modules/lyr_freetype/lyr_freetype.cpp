/* === S Y N F I G ========================================================= */
/*!	\file lyr_freetype.cpp
**	\brief Implementation of the "Text" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2006 Paul Wise
**	Copyright (c) 2007, 2008 Chris Moore
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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SYNFIG_LAYER

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "lyr_freetype.h"

#include <algorithm>
#include <glibmm.h>

#include FT_IMAGE_H
#include FT_OUTLINE_H

#if HAVE_HARFBUZZ
#include <fribidi.h>
#include <hb-ft.h>
#endif

#include <synfig/context.h>
#include <synfig/filesystemnative.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/rendering/common/task/taskcontour.h>
#include <synfig/string_helper.h>
#include "text_processing.h"
#include "fontloader.h"

#endif

using namespace synfig;


/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Freetype);
SYNFIG_LAYER_SET_NAME(Layer_Freetype,"text");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Freetype,N_("Text"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Freetype,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Freetype,"0.5");

extern FT_Library ft_library;


/* === C L A S S E S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_Freetype::Layer_Freetype()
	: face(nullptr)
{
#if HAVE_HARFBUZZ
	font = nullptr;
#endif
	param_size=ValueBase(Vector(0.25,0.25));
	param_text=ValueBase(std::string());//_("Text Layer"));
	param_color=ValueBase(Color::black());
	param_origin=ValueBase(Vector(0,0));
	param_orient=ValueBase(Vector(0.5,0.5));
	param_compress=ValueBase(Real(1.0));
	param_vcompress=ValueBase(Real(1.0));
	param_weight=ValueBase(TEXT_WEIGHT_NORMAL);
	param_style=ValueBase(TEXT_STYLE_NORMAL);
	param_direction=ValueBase(TEXT_DIRECTION_AUTO);
	param_family=ValueBase((const char*)"Sans Serif");
	param_use_kerning=ValueBase(true);
	param_grid_fit=ValueBase(false);
	param_invert=ValueBase(false);
	param_font=ValueBase(synfig::String());

	font_path_from_canvas = false;

	old_version=false;

	set_blend_method(Color::BLEND_COMPOSITE);
	need_sync=SYNC_FONT;

	synfig::String family=param_family.get(synfig::String());
	int style=param_style.get(int());
	int weight=param_weight.get(int());

	new_font(family,style,weight);

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();

	set_description(param_text.get(String()));
}

void
Layer_Freetype::on_canvas_set()
{
	Layer_Shape::on_canvas_set();

	synfig::String family=param_family.get(synfig::String());

	// Is it a font family or an absolute path for a font file? No need to reload it
	if (!FontLoader::has_valid_font_extension(family) || filesystem::Path::is_absolute_path(family))
		return;

	int style=param_style.get(int());
	int weight=param_weight.get(int());
	new_font(family,style,weight);

	sync(true);
}

void
Layer_Freetype::new_font(const synfig::String &family, int style, int weight)
{
	filesystem::Path canvas_path;
	if (get_canvas())
		canvas_path = get_canvas()->get_file_path();

	FontLoader::LoadedFont loaded = FontLoader::load_font(family, style, weight, canvas_path);
	if (!loaded)
		return;          // keep the previously loaded face rather than going faceless

	if (face != loaded.face)
		need_sync |= SYNC_FONT;

	face = loaded.face;
#if HAVE_HARFBUZZ
	font = loaded.font;
#endif
	font_path_from_canvas = loaded.path_from_canvas;
}

bool
Layer_Freetype::new_face(const String &newfont)
{
	int error = 0;
	FT_Long face_index=0;

	// If we are already loaded, don't bother reloading.
	if (face && param_font.get(synfig::String()) == newfont)
		return true;

	if(face)
		face = nullptr;

	std::string canvas_path;
	if (get_canvas())
		canvas_path = get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR;

	std::vector<std::string> filenames = FontLoader::get_possible_font_files(newfont, canvas_path);

	if (filenames.empty())
		return false;

	for (const std::string& path : filenames) {
		filesystem::Path absolute_path = filesystem::absolute(path);
		auto face_ptr = FontLoader::face_cache.get(absolute_path);
		if (face_ptr) {
			face = face_ptr;
#if HAVE_HARFBUZZ
			font = FaceMetaData::get_from_face(face).font;
#endif
			break;
		}
		error = FT_New_Face(ft_library, path.c_str(), face_index, &face);
		if (!error) {
			FontLoader::face_cache.put(absolute_path, face);
#if HAVE_HARFBUZZ
			font = hb_ft_font_create(face, nullptr);
			FaceMetaData::add_to_face(face, path, font);
#else
			FaceMetaData::add_to_face(face, path);
#endif
			font_path_from_canvas = !canvas_path.empty() && path.compare(0, canvas_path.size(), canvas_path) == 0;
			break;
		}
	}

	if(error)
	{
		synfig::error(strprintf("Layer_Freetype: %s (err=%d): %s",_("Unable to open font face."),error,newfont.c_str()));
		return false;
	}

	need_sync |= SYNC_FONT;
	return true;
}

bool
Layer_Freetype::set_simple_shape_param(const synfig::String &param, const synfig::ValueBase &value)
{
	std::lock_guard<std::mutex> lock(mutex);

	IMPORT_VALUE_PLUS(param_size,
		{
			if(old_version)
			{
				synfig::Vector size=param_size.get(synfig::Vector());
				size/=2.0;
				param_size.set(size);
			}
		}
		);

	return false;
}

bool
Layer_Freetype::set_shape_param(const String & param, const ValueBase &value)
{
	std::lock_guard<std::mutex> lock(mutex);
/*
	if(param=="font" && value.same_type_as(font))
	{
		new_font(filesystem::Path::basename(value.get(font)),style,weight);
		family=filesystem::Path::basename(value.get(font));
		return true;
	}
*/
	IMPORT_VALUE_PLUS(param_family,
		{
			synfig::String family = FileSystem::fix_slashes(value.get(synfig::String()));
			param_family.set(family);
			int style=param_style.get(int());
			int weight=param_weight.get(int());
			new_font(family,style,weight);
		}
		);

	IMPORT_VALUE_PLUS(param_weight,
		{
			synfig::String family=param_family.get(synfig::String());
			int style=param_style.get(int());
			int weight=param_weight.get(int());
			new_font(family,style,weight);
		}
		);
	IMPORT_VALUE_PLUS(param_style,
		{
			synfig::String family=param_family.get(synfig::String());
			int style=param_style.get(int());
			int weight=param_weight.get(int());
			new_font(family,style,weight);
		}
		);
	IMPORT_VALUE_PLUS(param_direction,need_sync |= SYNC_DIRECTION);
	IMPORT_VALUE_PLUS(param_text,
		{
			on_param_text_changed();
		}
		);
	IMPORT_VALUE_PLUS(param_orient,need_sync |= SYNC_ORIENTATION;);
	IMPORT_VALUE_PLUS(param_compress,need_sync |= SYNC_COMPRESS);
	IMPORT_VALUE_PLUS(param_vcompress,need_sync |= SYNC_COMPRESS);
	IMPORT_VALUE_PLUS(param_use_kerning,need_sync |= SYNC_KERNING);
	IMPORT_VALUE_PLUS(param_grid_fit,need_sync |= SYNC_GRID_FIT);

	if(param=="pos")
		return set_param("origin", value);

	return false;
}

bool
Layer_Freetype::set_param(const String & param, const ValueBase &value)
{
	if (set_simple_shape_param(param, value))
		return true;

	return Layer_Shape::set_param(param, value);
}

ValueBase
Layer_Freetype::get_param(const String& param)const
{
	EXPORT_VALUE(param_font);
	EXPORT_VALUE(param_family);
	EXPORT_VALUE(param_style);
	EXPORT_VALUE(param_weight);
	EXPORT_VALUE(param_direction);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_text);
	EXPORT_VALUE(param_orient);
	EXPORT_VALUE(param_compress);
	EXPORT_VALUE(param_vcompress);
	EXPORT_VALUE(param_use_kerning);
	EXPORT_VALUE(param_grid_fit);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab
Layer_Freetype::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());
	// Ignore the Layer_Shape params

	ret.push_back(ParamDesc("text")
		.set_local_name(_("Text"))
		.set_description(_("Text to Render"))
		.set_hint("paragraph")
	);

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Color of the text"))
	);

	ret.push_back(ParamDesc("family")
		.set_local_name(_("Font Family"))
		.set_description(_("You can select or type a font family name or the font file path"))
		.set_hint("font_family")
	);

	ret.push_back(ParamDesc("style")
		.set_local_name(_("Style"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(TEXT_STYLE_NORMAL, "normal" ,_("Normal"))
		.add_enum_value(TEXT_STYLE_OBLIQUE, "oblique" ,_("Oblique"))
		.add_enum_value(TEXT_STYLE_ITALIC, "italic" ,_("Italic"))
	);

	ret.push_back(ParamDesc("weight")
		.set_local_name(_("Weight"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(TEXT_WEIGHT_THIN, "thin" ,_("Thin"))
		.add_enum_value(TEXT_WEIGHT_ULTRALIGHT, "ultralight" ,_("Ultralight"))
		.add_enum_value(TEXT_WEIGHT_LIGHT, "light" ,_("Light"))
		.add_enum_value(TEXT_WEIGHT_BOOK, "book" ,_("Book"))
		.add_enum_value(TEXT_WEIGHT_NORMAL, "normal" ,_("Normal"))
		.add_enum_value(TEXT_WEIGHT_MEDIUM, "medium" ,_("Medium"))
		.add_enum_value(TEXT_WEIGHT_BOLD, "bold" ,_("Bold"))
		.add_enum_value(TEXT_WEIGHT_ULTRABOLD, "ultrabold" ,_("Ultrabold"))
		.add_enum_value(TEXT_WEIGHT_HEAVY, "heavy" ,_("Heavy"))
		.add_enum_value(TEXT_WEIGHT_ULTRAHEAVY, "ultraheavy" ,_("Ultraheavy"))
	);

	ret.push_back(ParamDesc("direction")
		.set_local_name(_("Direction"))
		.set_description(_("The text direction: left-to-right or right-to-left"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(TEXT_DIRECTION_AUTO, "auto" ,_("Automatic"))
		.add_enum_value(TEXT_DIRECTION_LTR, "ltr" ,_("LTR"))
		.add_enum_value(TEXT_DIRECTION_RTL, "rtl" ,_("RTL"))
	);

	ret.push_back(ParamDesc("compress")
		.set_local_name(_("Horizontal Spacing"))
		.set_description(_("Defines how close the glyphs are horizontally"))
	);

	ret.push_back(ParamDesc("vcompress")
		.set_local_name(_("Vertical Spacing"))
		.set_description(_("Defines how close the text lines are vertically"))
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the text"))
		.set_hint("size")
		.set_origin("origin")
		.set_is_distance()
	);

	ret.push_back(ParamDesc("orient")
		.set_local_name(_("Orientation"))
		.set_description(_("Text Orientation"))
		.set_invisible_duck()
	);

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Text Position"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("font")
		.set_local_name(_("Font"))
		.set_description(_("Filename of the font to use"))
		.set_hint("filename")
		.not_critical()
		.hidden()
	);

	ret.push_back(ParamDesc("use_kerning")
		.set_local_name(_("Kerning"))
		.set_description(_("When checked, enables font kerning (If the font supports it)"))
	);

	ret.push_back(ParamDesc("grid_fit")
		.set_local_name(_("Sharpen Edges"))
		.set_description(_("Turn this off if you are animating the text"))
	);

	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert"))
	);
	return ret;
}

void
Layer_Freetype::sync_vfunc()
{
	std::lock_guard<std::mutex> lock(sync_mtx);

	clear();

	std::string text = param_text.get(std::string());

	if (synfig::trim(text).empty() || !face) {
		lines.clear();
		return;
	}

	const bool use_kerning = param_use_kerning.get(bool());
	const bool grid_fit    = param_grid_fit.get(bool());
	const Vector orient    = param_orient.get(Vector());
	const Real compress    = param_compress.get(Real());
	const Real vcompress   = param_vcompress.get(Real());
	const int direction    = param_direction.get(0);

	if(text=="@_FILENAME_@" && get_canvas() && !get_canvas()->get_file_name().empty())
	{
		auto text = filesystem::Path::basename(get_canvas()->get_file_name());
		lines = text_processing::fetch_text_lines(text, direction);
	}

#if HAVE_HARFBUZZ
	hb_buffer_t *span_buffer = hb_buffer_create();
	std::unique_ptr<hb_buffer_t, decltype(&hb_buffer_destroy)> safe_buf(span_buffer, hb_buffer_destroy); // auto delete
#endif

	// Lines of glyph indices
	// Depends on: font and text
	std::vector<std::vector<uint32_t>> glyph_indices;

	for (const TextLine& line : lines)
	{
		std::vector<uint32_t> glyph_index_line;

		for (const TextSpan& span : line) {
#if HAVE_HARFBUZZ
			hb_buffer_clear_contents(span_buffer);

			hb_direction_t direction = span.direction;
			hb_buffer_set_direction(span_buffer, direction);
			hb_buffer_set_script(span_buffer, span.script);
//			hb_buffer_set_language(span_buffer, hb_language_from_string(language.c_str(), -1));

			hb_buffer_add_utf32(span_buffer, span.codepoints.data(), span.codepoints.size(), 0, -1);

			hb_shape(font, span_buffer, nullptr, 0);

			unsigned int glyph_count;
			hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(span_buffer, &glyph_count);
#else
			size_t glyph_count = span.codepoints.size();
#endif

			for (size_t i = 0; i < glyph_count; i++) {
				uint32_t glyph_index;
#if HAVE_HARFBUZZ
				glyph_index = glyph_info[i].codepoint;
#else
				glyph_index = FT_Get_Char_Index(face, span.codepoints[i]);
#endif
				glyph_index_line.push_back(glyph_index);
			}
		}

		glyph_indices.push_back(glyph_index_line);
	}

	// get visual info
	// Depends on: glyph indices, font and grid_fit
	struct Glyph {
		Vector advance;
		FT_BBox bbox;
		rendering::Contour::ChunkList outline;
	};

	std::map<uint32_t, Glyph> glyph_map;

	for (const std::vector<uint32_t>& glyph_line : glyph_indices)
	{
		for (const uint32_t glyph_index : glyph_line) {
			if (glyph_map.count(glyph_index))
				continue;

			// load glyph image into the slot. DO NOT RENDER IT !!
			FT_Error error;
			if(grid_fit)
				error = FT_Load_Glyph( face, glyph_index, FT_LOAD_NO_SCALE);
			else
				error = FT_Load_Glyph( face, glyph_index, FT_LOAD_NO_SCALE|FT_LOAD_NO_HINTING );
			if (error) continue;  // ignore errors, jump to next glyph

			// extract glyph image and store it in our table
			FT_Glyph ftglyph;
			error = FT_Get_Glyph( face->glyph, &ftglyph );
			if (error) continue;  // ignore errors, jump to next glyph

			Glyph glyph;
			glyph.advance = Vector(ftglyph->advance.x >> 10, ftglyph->advance.y >> 10);
			FT_Glyph_Get_CBox(ftglyph, ft_glyph_bbox_subpixels, &glyph.bbox);

			FT_OutlineGlyph outline_glyph = nullptr;
			if (ftglyph->format == FT_GLYPH_FORMAT_OUTLINE) {
				outline_glyph = FT_OutlineGlyph(ftglyph);
				text_processing::convert_outline_to_contours(&outline_glyph->outline, glyph.outline);
			}

			glyph_map[glyph_index] = glyph;

			FT_Done_Glyph(ftglyph);
		}
	}

	// Now 'render' and get the metrics
	// Depends on: font, kerning, compress, vcompress
	std::vector<rendering::Contour::ChunkList> visual_text;
	std::vector<Real> line_widths;
	Real initial_y = 0;

	Vector offset;
	const FT_UInt kern_mode = grid_fit ? FT_KERNING_DEFAULT : FT_KERNING_UNFITTED;
	for (const std::vector<uint32_t>& glyph_line : glyph_indices)
	{
		uint32_t previous_glyph_index = 0;
		offset[0] = 0;
		rendering::Contour::ChunkList visual_line;

		for (const uint32_t glyph_index : glyph_line) {

			// retrieve kerning distance and move pen position
			if ( use_kerning && previous_glyph_index && glyph_index && FT_HAS_KERNING(face) )
			{
				FT_Vector delta;
				FT_Error error;
				error = FT_Get_Kerning( face, previous_glyph_index, glyph_index, kern_mode, &delta );
				if (!error) {
					offset[0] += delta.x*compress;
					offset[1] += delta.y*compress;
				}
			}

			// 'render' the glyph
			try {
				const Glyph &glyph = glyph_map.at(glyph_index);

				rendering::Contour::ChunkList chunks = glyph.outline;
				text_processing::shift_contour_chunks(chunks, offset);
				visual_line.insert(visual_line.end(), std::make_move_iterator(chunks.begin()), std::make_move_iterator(chunks.end()));

				if (visual_text.empty()) { // First line?
					initial_y = std::max(initial_y, Real(glyph.bbox.yMax));
				}

				offset[0] += glyph.advance[0] * compress;
				offset[1] += glyph.advance[1];
			} catch (std::out_of_range &ex) {
				continue;
			}

			previous_glyph_index = glyph_index;
		}

		offset[1] -= face->height * vcompress;
		line_widths.push_back(offset[0]);

		visual_text.push_back(visual_line);
	}

	// Add contour chunks to shape
	// Depends on: orientation

	const Real text_height = initial_y + (visual_text.size() - 1) * vcompress * face->height;

	for (size_t i = 0; i < visual_text.size(); i++) {
		auto& visual_line = visual_text[i];
		Vector offset;
		offset[0] = - orient[0] * line_widths[i];
		offset[1] =   orient[1] * text_height - initial_y;
		text_processing::shift_contour_chunks(visual_line, offset);
		add(visual_line);
	}
}

bool
Layer_Freetype::is_inside_contour(const Point& p, bool ignore_feather) const
{
	sync();

	const Point point_in_contour = world_to_contour(p);

	return Layer_Shape::is_inside_contour(point_in_contour, ignore_feather);
}

Rect
Layer_Freetype::get_bounding_rect() const
{
	sync();

	Rect bounds = Layer_Shape::get_bounding_rect();

	bounds = Rect(contour_to_world(bounds.get_min()), contour_to_world(bounds.get_max()));

	return bounds;
}

void
Layer_Freetype::on_param_text_changed()
{
	std::lock_guard<std::mutex> lock(sync_mtx);

	lines = text_processing::fetch_text_lines(param_text.get(std::string()), param_direction.get(0));

	need_sync |= SYNC_TEXT;
}

Point
Layer_Freetype::world_to_contour(const synfig::Point &p) const
{
	if (!face)
		return p;
	Vector size = param_size.get(Vector()) * 2;

	// Multiplies by face->units_per_EM to avoid rounding errors due to matrix inversion
	// (face->units_per_EM is usually higher than 2000)
	const Vector& t = param_origin.get(Vector()) * face->units_per_EM;

	Matrix matrix(
			size[0], 0,       0,
			0,       size[1], 0,
			t[0],    t[1],    face->units_per_EM);

	return (matrix.get_inverted()*face->units_per_EM).get_transformed(p);
}

Point Layer_Freetype::contour_to_world(const synfig::Point &p) const
{
	if (!face)
		return p;
	Vector size = param_size.get(Vector()) * 2;

	Matrix matrix = Matrix().set_translate(param_origin.get(Vector()))
					* Matrix().set_scale(size/(face->units_per_EM));

	return matrix.get_transformed(p);
}

rendering::Task::Handle
Layer_Freetype::build_composite_task_vfunc(ContextParams context_params) const
{
	rendering::Task::Handle task = Layer_Shape::build_composite_task_vfunc(context_params);

	if (!face)
		return task;

	Vector size(param_size.get(synfig::Vector())*2);
	Matrix matrix = Matrix().set_translate(param_origin.get(Vector()))
					* Matrix().set_scale(size/(face->units_per_EM));

	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->sub_task() = task;
//	task_transformation->interpolation = Color::INTERPOLATION_LINEAR;
	task_transformation->transformation->matrix = matrix;

	task = task_transformation;
	return task;
}
