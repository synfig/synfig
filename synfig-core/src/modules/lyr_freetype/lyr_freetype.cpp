/* === S Y N F I G ========================================================= */
/*!	\file lyr_freetype.cpp
**	\brief Implementation of the "Text" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2006 Paul Wise
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

/* === H E A D E R S ======================================================= */

#define SYNFIG_LAYER

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif
#ifdef WITH_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#include "lyr_freetype.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/canvasfilenaming.h>

#include <synfig/context.h>

#include <algorithm>
#include <glibmm.h>

#endif

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define MAX_GLYPHS		2000

// Copy of PangoStyle
// It is necessary to keep original values if Pango ever change them
//  - because it would change layer rendering as Synfig stores the parameter
//     value as an integer (ie. not a weight name string)
enum TextStyle{
	TEXT_STYLE_NORMAL = 0,
	TEXT_STYLE_OBLIQUE = 1,
	TEXT_STYLE_ITALIC = 2
};

// Copy of PangoWeight
// It is necessary to keep original values if Pango ever change them
//  - because it would change layer rendering as Synfig stores the parameter
//     value as an integer (ie. not a weight name string)
enum TextWeight{
	TEXT_WEIGHT_THIN = 100,
	TEXT_WEIGHT_ULTRALIGHT = 200,
	TEXT_WEIGHT_LIGHT = 300,
	TEXT_WEIGHT_SEMILIGHT = 350,
	TEXT_WEIGHT_BOOK = 380,
	TEXT_WEIGHT_NORMAL = 400,
	TEXT_WEIGHT_MEDIUM = 500,
	TEXT_WEIGHT_SEMIBOLD = 600,
	TEXT_WEIGHT_BOLD = 700,
	TEXT_WEIGHT_ULTRABOLD = 800,
	TEXT_WEIGHT_HEAVY = 900,
	TEXT_WEIGHT_ULTRAHEAVY = 1000
};

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Freetype);
SYNFIG_LAYER_SET_NAME(Layer_Freetype,"text");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Freetype,N_("Text"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Freetype,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Freetype,"0.3");

#ifndef __APPLE__
static const std::vector<const char *> known_font_extensions = {".ttf", ".otf", ".ttc"};
#else
static const std::vector<const char *> known_font_extensions = {".ttf", ".otf", ".dfont", ".ttc"};
#endif

extern FT_Library ft_library;

/* === C L A S S E S ======================================================= */

struct Glyph
{
	FT_Glyph glyph;
	FT_Vector pos;
};

struct TextLine
{
	int width;
	std::vector<Glyph> glyph_table;

	TextLine():width(0) { }

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

#ifdef WITH_FONTCONFIG
// Allow proper finalization of FontConfig
struct FontConfigWrap {
	static FcConfig* init() {
		static FontConfigWrap obj;
		return obj.config;
	}

	FontConfigWrap(FontConfigWrap const&) = delete;
	void operator=(FontConfigWrap const&) = delete;
private:
	FcConfig* config = nullptr;

	FontConfigWrap()
	{
		config = FcInitLoadConfigAndFonts();
#ifdef _WIN32
		// Windows 10 (1809) Added local user fonts installed to C:\Users\%USERNAME%\AppData\Local\Microsoft\Windows\Fonts
		std::string localdir = Glib::getenv("LOCALAPPDATA");
		if (!localdir.empty()) {
			localdir.append("\\Microsoft\\Windows\\Fonts\\");
			FcConfigAppFontAddDir(config, (const FcChar8 *)localdir.c_str());
		}
#endif
	}
	~FontConfigWrap() {
		FcConfigDestroy(config);
		config = nullptr;
	}
};

static std::string fontconfig_get_filename(const std::string& font_fam, int style, int weight);
#endif

/// Metadata about a font. Used for font face cache indexing
struct FontMeta {
	synfig::String family;
	int style;
	int weight;
	//! Canvas file path if loaded font face file depends on it.
	//!  Empty string otherwise
	std::string canvas_path;

	FontMeta(synfig::String family, int style=0, int weight=400)
		: family(family), style(style), weight(weight)
	{}

	bool operator==(const FontMeta& other) const
	{
		return family == other.family && style == other.style && weight == other.weight && canvas_path == other.canvas_path;
	}

	bool operator<(const FontMeta& other) const
	{
		if (family < other.family)
			return true;
		if (family != other.family)
			return false;

		if (style < other.style)
			return true;
		if (style > other.style)
			return false;

		if (weight < other.weight)
			return true;
		if (weight > other.weight)
			return false;

		if (canvas_path < other.canvas_path)
			return true;

		return false;
	}
};

/// Cache font faces for speeding up the text layer rendering
class FaceCache {
	std::map<FontMeta, FT_Face> cache;
public:
	FT_Face get(const FontMeta &meta) const {
		auto iter = cache.find(meta);
		if (iter != cache.end())
			return iter->second;
		return nullptr;
	}

	void put(const FontMeta &meta, FT_Face face) {
		cache[meta] = face;
	}

	bool has(const FontMeta &meta) const {
		auto iter = cache.find(meta);
		return iter != cache.end();
	}

	void clear() {
		for (auto item : cache)
			FT_Done_Face(item.second);
		cache.clear();
	}

	static FaceCache& instance() {
		static FaceCache obj;
		return obj;
	}

private:
	FaceCache() {}
	FaceCache(const FaceCache&) = delete;

	~FaceCache() {
		clear();
	}
};

/* === P R O C E D U R E S ================================================= */

static bool
has_valid_font_extension(const std::string &filename) {
	std::string extension = etl::filename_extension(filename);
	return std::find(known_font_extensions.begin(), known_font_extensions.end(), extension) != known_font_extensions.end();
}

/// Try to map a font family to a filename (without extension nor directory)
static void
get_possible_font_filenames(synfig::String family, int style, int weight, std::vector<std::string>& list)
{
	// string :: tolower
	std::transform(family.begin(), family.end(), family.begin(),
		[](unsigned char c){ return std::tolower(c); });

	enum FontSuffixStyle {FONT_SUFFIX_NONE, FONT_SUFFIX_BI_BD, FONT_SUFFIX_BI_BD_IT, FONT_SUFFIX_BI_RI};
	enum FontClassification {FONT_SANS_SERIF, FONT_SERIF, FONT_MONOSPACED, FONT_SCRIPT};

	struct FontFileNameEntry {
		const char *alias;
		const char *preffix;
		const char *alternative_preffix;
		FontSuffixStyle suffix_style;
		FontClassification classification;

		std::string get_suffix(int style, int weight) const {
			std::string suffix;
			switch (suffix_style) {
			case FONT_SUFFIX_NONE:
				break;
			case FONT_SUFFIX_BI_BD:
				if (weight>TEXT_WEIGHT_NORMAL)
					suffix+='b';
				if (style==TEXT_STYLE_ITALIC || style==TEXT_STYLE_OBLIQUE)
					suffix+='i';
				else if (weight>TEXT_WEIGHT_NORMAL)
					suffix+='d';
				break;
			case FONT_SUFFIX_BI_BD_IT:
				if (weight>TEXT_WEIGHT_NORMAL)
					suffix+='b';
				if (style==TEXT_STYLE_ITALIC || style==TEXT_STYLE_OBLIQUE)
				{
					suffix+='i';
					if (weight<=TEXT_WEIGHT_NORMAL)
						suffix+='t';
				}
				else if(weight>TEXT_WEIGHT_NORMAL)
					suffix+='d';
				break;
			case FONT_SUFFIX_BI_RI:
				if(weight>TEXT_WEIGHT_NORMAL)
					suffix+='b';
				else
					suffix+='r';
				if(style==TEXT_STYLE_ITALIC || style==TEXT_STYLE_OBLIQUE)
					suffix+='i';
				break;
			}
			return suffix;
		}

		static std::string get_alternative_suffix(int style, int weight) {
			if (weight > TEXT_WEIGHT_NORMAL) {
				if (style == TEXT_STYLE_ITALIC)
					return " Bold Italic";
				else if (style == TEXT_STYLE_OBLIQUE)
					return " Bold Oblique";
				else
					return " Bold";
			} else {
				if (style == TEXT_STYLE_ITALIC)
					return " Italic";
				else if (style == TEXT_STYLE_OBLIQUE)
					return " Oblique";
				else
					return "";
			}
		}

	};

	struct SpecialFontFamily {
		const char * const alias;
		const char * const option1;
		const char * const option2;
		const char * const option3;
	};

	const SpecialFontFamily special_font_family_db[] = {
		{"sans serif", "arial", "luxi sans", "helvetica"},
		{"serif", "times new roman", "luxi serif", nullptr},
		{"comic", "comic sans", nullptr, nullptr},
		{"courier", "courier new", nullptr, nullptr},
		{"times", "times new roman", nullptr, nullptr},
		{nullptr, nullptr, nullptr, nullptr}
	};

	const FontFileNameEntry font_filename_db[] = {
		{"arial black", "ariblk", nullptr, FONT_SUFFIX_NONE, FONT_SANS_SERIF},
		{"arial", "arial", "Arial", FONT_SUFFIX_BI_BD, FONT_SANS_SERIF},
		{"comic sans", "comic", nullptr, FONT_SUFFIX_BI_BD, FONT_SANS_SERIF},
		{"courier new", "cour", "Courier New", FONT_SUFFIX_BI_BD, FONT_MONOSPACED},
		{"times new roman", "times", "Times New Roman", FONT_SUFFIX_BI_BD, FONT_SERIF},
		{"trebuchet", "trebuc", "Trebuchet MS", FONT_SUFFIX_BI_BD_IT, FONT_SANS_SERIF},
		{"luxi sans", "luxis", nullptr, FONT_SUFFIX_BI_RI, FONT_SANS_SERIF},
		{"luxi serif", "luxir", nullptr, FONT_SUFFIX_BI_RI, FONT_SERIF},
		{"luxi mono", "luxim", nullptr, FONT_SUFFIX_BI_RI, FONT_MONOSPACED},
		{"luxi", "luxim", nullptr, FONT_SUFFIX_BI_RI, FONT_MONOSPACED},
		{nullptr, nullptr, nullptr, FONT_SUFFIX_NONE, FONT_SANS_SERIF},
	};

	std::vector<std::string> possible_families;
	for (int i = 0; special_font_family_db[i].alias; i++) {
		const SpecialFontFamily &special_family = special_font_family_db[i];
		if (special_family.alias == family) {
			possible_families.push_back(special_family.option1);
			if (special_family.option2) {
				possible_families.push_back(special_family.option2);
				if (special_family.option3)
					possible_families.push_back(special_family.option3);
			}
			break;
		}
	}
	if (possible_families.empty())
		possible_families.push_back(family);

	for (const std::string &possible_family : possible_families) {
		for (int i = 0; font_filename_db[i].alias; i++) {
			const FontFileNameEntry &entry = font_filename_db[i];
			if (possible_family == entry.alias) {
				std::string filename = entry.preffix;
				filename += entry.get_suffix(style, weight);
				list.push_back(filename);

				filename = entry.preffix;
				filename += entry.get_alternative_suffix(style, weight);
				list.push_back(filename);
			}
		}
	}
}

/* === M E T H O D S ======================================================= */

Layer_Freetype::Layer_Freetype()
	: face(nullptr)
{
	param_size=ValueBase(Vector(0.25,0.25));
	param_text=ValueBase(_("Text Layer"));
	param_color=ValueBase(Color::black());
	param_origin=ValueBase(Vector(0,0));
	param_orient=ValueBase(Vector(0.5,0.5));
	param_compress=ValueBase(Real(1.0));
	param_vcompress=ValueBase(Real(1.0));
	param_weight=ValueBase(TEXT_WEIGHT_NORMAL);
	param_style=ValueBase(TEXT_STYLE_NORMAL);
	param_family=ValueBase((const char*)"Sans Serif");
	param_use_kerning=ValueBase(true);
	param_grid_fit=ValueBase(false);
	param_invert=ValueBase(false);
	param_font=ValueBase(synfig::String());

	font_path_from_canvas = false;

	old_version=false;

	set_blend_method(Color::BLEND_COMPOSITE);
	needs_sync_=true;

	synfig::String family=param_family.get(synfig::String());
	int style=param_style.get(int());
	int weight=param_weight.get(int());

	new_font(family,style,weight);

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();

	set_description(param_text.get(String()));
}

Layer_Freetype::~Layer_Freetype()
{
}

void
Layer_Freetype::on_canvas_set()
{
	Layer_Composite::on_canvas_set();

	synfig::String family=param_family.get(synfig::String());

	// Is it a font family or an absolute path for a font file? No need to reload it
	if (!has_valid_font_extension(family) || etl::is_absolute_path(family))
		return;

	int style=param_style.get(int());
	int weight=param_weight.get(int());
	new_font(family,style,weight);
}

/*! The new_font() function try to render
**	text until it work by simplyfing font style(s).
** In last chance, render text as "sans serif" Normal
** font style.
*/
void
Layer_Freetype::new_font(const synfig::String &family, int style, int weight)
{
	if(
		!new_font_(family,style,weight) &&
		!new_font_(family,style,TEXT_WEIGHT_NORMAL) &&
		!new_font_(family,TEXT_STYLE_NORMAL,weight) &&
		!new_font_(family,TEXT_STYLE_NORMAL,TEXT_WEIGHT_NORMAL) &&
		!new_font_("sans serif",style,weight) &&
		!new_font_("sans serif",style,TEXT_WEIGHT_NORMAL) &&
		!new_font_("sans serif",TEXT_STYLE_NORMAL,weight)
	)
		new_font_("sans serif",TEXT_STYLE_NORMAL,TEXT_WEIGHT_NORMAL);
}

bool
Layer_Freetype::new_font_(const synfig::String &font_fam_, int style, int weight)
{
	FontMeta meta(font_fam_, style, weight);
	if (get_canvas())
		meta.canvas_path = get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR;

	FaceCache &face_cache = FaceCache::instance();

	FT_Face tmp_face = face_cache.get(meta);
	if (tmp_face) {
		face = tmp_face;
		return true;
	}

	synfig::String font_fam(font_fam_);

	if (has_valid_font_extension(font_fam_))
		if (new_face(font_fam_)) {
			if (!font_path_from_canvas)
				meta.canvas_path.clear();
			face_cache.put(meta, face);
			return true;
		}

#ifdef WITH_FONTCONFIG
	if (new_face(fontconfig_get_filename(font_fam_, style, weight))) {
		if (!font_path_from_canvas)
			meta.canvas_path.clear();
		face_cache.put(meta, face);
		return true;
	}
#endif

	std::vector<std::string> filename_list;
	get_possible_font_filenames(font_fam_, style, weight, filename_list);

	for (std::string& filename : filename_list) {
		if (new_face(filename)) {
			if (!font_path_from_canvas)
				meta.canvas_path.clear();
			face_cache.put(meta, face);
			return true;
		}
	}
	if (new_face(font_fam_)) {
		if (!font_path_from_canvas)
			meta.canvas_path.clear();
		face_cache.put(meta, face);
		return true;
	}

	return false;
}

#ifdef WITH_FONTCONFIG

static std::string fontconfig_get_filename(const std::string& font_fam, int style, int weight) {
	std::string filename;
	FcConfig* fc = FontConfigWrap::init();
	if( !fc )
	{
		synfig::warning("Layer_Freetype: fontconfig: %s",_("unable to initialize"));
	} else {
		FcPattern* pat = FcPatternCreate();
		FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)font_fam.c_str());
		FcPatternAddInteger(pat, FC_SLANT, style == TEXT_STYLE_NORMAL ? FC_SLANT_ROMAN : (style == TEXT_STYLE_ITALIC ? FC_SLANT_ITALIC : FC_SLANT_OBLIQUE));
		int fc_weight;
#define SYNFIG_TO_FC(X) TEXT_WEIGHT_##X : fc_weight = FC_WEIGHT_##X ; break
		switch (weight) {
		case SYNFIG_TO_FC(NORMAL);
		case SYNFIG_TO_FC(BOLD);
		case SYNFIG_TO_FC(THIN);
		case SYNFIG_TO_FC(ULTRALIGHT);
		case SYNFIG_TO_FC(LIGHT);
		case SYNFIG_TO_FC(SEMILIGHT);
		case SYNFIG_TO_FC(BOOK);
		case SYNFIG_TO_FC(MEDIUM);
		case SYNFIG_TO_FC(SEMIBOLD);
		case SYNFIG_TO_FC(ULTRABOLD);
		case SYNFIG_TO_FC(HEAVY);
		case TEXT_WEIGHT_ULTRAHEAVY : fc_weight = FC_WEIGHT_HEAVY ; break;
		default:
			fc_weight = FC_WEIGHT_NORMAL;
		}
#undef SYNFIG_TO_FC
		FcPatternAddInteger(pat, FC_WEIGHT, fc_weight);

		FcConfigSubstitute(fc, pat, FcMatchPattern);
		FcDefaultSubstitute(pat);
		FcFontSet *fs = FcFontSetCreate();
		FcResult result;
		FcPattern *match = FcFontMatch(fc, pat, &result);
		if (match)
			FcFontSetAdd(fs, match);
		if (pat)
			FcPatternDestroy(pat);
		if(fs && fs->nfont){
			FcChar8* file;
			if( FcPatternGetString (fs->fonts[0], FC_FILE, 0, &file) == FcResultMatch )
				filename = (const char*)file;
			FcFontSetDestroy(fs);
		} else
			synfig::warning("Layer_Freetype: fontconfig: %s",_("empty font set"));
	}
	return filename;
}
#endif

bool
Layer_Freetype::new_face(const String &newfont)
{
	synfig::String font=param_font.get(synfig::String());
	int error = 0;
	FT_Long face_index=0;

	// If we are already loaded, don't bother reloading.
	if(face && font==newfont)
		return true;

	if(face)
		face = nullptr;

	if (newfont.empty())
		return false;

	std::vector<const char *> possible_font_extensions = {""};

	// if newfont doesn't have a known extension, try to append those extensions
	if (! has_valid_font_extension(newfont))
		possible_font_extensions.insert(possible_font_extensions.end(), known_font_extensions.begin(), known_font_extensions.end());

	std::string canvas_path;
	if (get_canvas())
		canvas_path = get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR;

	std::vector<std::string> possible_font_directories = get_possible_font_directories(canvas_path);

	for (std::string directory : possible_font_directories) {
		for (const char *extension : possible_font_extensions) {
			std::string path = (directory + newfont + extension);
			error = FT_New_Face(ft_library, path.c_str(), face_index, &face);
			if (!error) {
				font_path_from_canvas = !canvas_path.empty() && directory == canvas_path;
				break;
			}
		}
		if (!error)
			break;
	}

	if(error)
	{
		if (!newfont.empty())
			synfig::error(strprintf("Layer_Freetype: %s (err=%d): %s",_("Unable to open font face."),error,newfont.c_str()));
		return false;
	}

	// ???
	font=newfont;

	needs_sync_=true;
	return true;
}

std::vector<std::string>
Layer_Freetype::get_possible_font_directories(const std::string& canvas_path)
{
	std::vector<std::string> possible_font_directories = {""};

	if (!canvas_path.empty())
		possible_font_directories.push_back(canvas_path);

#ifdef _WIN32
	// All users fonts
	std::string windir = Glib::getenv("windir");
	if (windir.empty()) {
		possible_font_directories.emplace_back("C:\\WINDOWS\\FONTS\\");
	} else {
		possible_font_directories.emplace_back(windir + "\\Fonts\\");
	}
	// Windows 10 (1809) Added local user fonts installed to C:\Users\%USERNAME%\AppData\Local\Microsoft\Windows\Fonts
	std::string localdir = Glib::getenv("LOCALAPPDATA");
	if (!localdir.empty()) {
		possible_font_directories.emplace_back(localdir + "\\Microsoft\\Windows\\Fonts\\");
	}
#else

#ifdef __APPLE__
	std::string userdir = Glib::getenv("HOME");
	possible_font_directories.push_back(userdir + "/Library/Fonts/");
	possible_font_directories.push_back("/Library/Fonts/");
#endif

	possible_font_directories.push_back("/usr/share/fonts/truetype/");
	possible_font_directories.push_back("/usr/share/fonts/opentype/");

#endif

	return possible_font_directories;
}

bool
Layer_Freetype::set_param(const String & param, const ValueBase &value)
{
	std::lock_guard<std::mutex> lock(mutex);
/*
	if(param=="font" && value.same_type_as(font))
	{
		new_font(etl::basename(value.get(font)),style,weight);
		family=etl::basename(value.get(font));
		return true;
	}
*/
	IMPORT_VALUE_PLUS(param_family,
		{
			synfig::String family=param_family.get(synfig::String());
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
	IMPORT_VALUE_PLUS(param_size,
		{
			if(old_version)
			{
				synfig::Vector size=param_size.get(synfig::Vector());
				size/=2.0;
				param_size.set(size);
			}
			needs_sync_=true;
		}
		);
	IMPORT_VALUE_PLUS(param_text,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_origin,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_color,
		{
			Color color=param_color.get(Color());
			if (approximate_zero(color.get_a()))
			{
				if (converted_blend_)
				{
					set_blend_method(Color::BLEND_ALPHA_OVER);
					color.set_a(1);
					param_color.set(color);
				} else transparent_color_ = true;
			}
		}
		);
	IMPORT_VALUE(param_invert);
	IMPORT_VALUE_PLUS(param_orient,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_compress,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_vcompress,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_use_kerning,needs_sync_=true);
	IMPORT_VALUE_PLUS(param_grid_fit,needs_sync_=true);

	if(param=="pos")
		return set_param("origin", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Freetype::get_param(const String& param)const
{
	EXPORT_VALUE(param_font);
	EXPORT_VALUE(param_family);
	EXPORT_VALUE(param_style);
	EXPORT_VALUE(param_weight);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_text);
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_orient);
	EXPORT_VALUE(param_compress);
	EXPORT_VALUE(param_vcompress);
	EXPORT_VALUE(param_use_kerning);
	EXPORT_VALUE(param_grid_fit);
	EXPORT_VALUE(param_invert);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Layer_Freetype::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

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
Layer_Freetype::sync()
{
	needs_sync_=false;
}

inline Color
Layer_Freetype::color_func(const Point &/*point_*/, int /*quality*/, ColorReal /*supersample*/)const
{
	bool invert=param_invert.get(bool());
	if (invert)
		return param_color.get(Color());
	else
		return Color::alpha();
}

Color
Layer_Freetype::get_color(Context context, const synfig::Point &pos)const
{
	if(needs_sync_)
		const_cast<Layer_Freetype*>(this)->sync();

	if(!face)
		return context.get_color(pos);

	const Color color(color_func(pos,0));

	if(get_amount()==1.0f && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}

bool
Layer_Freetype::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	bool use_kerning=param_use_kerning.get(bool());
	bool grid_fit=param_grid_fit.get(bool());
	bool invert=param_invert.get(bool());
	Color color=param_color.get(Color());
	synfig::Point origin=param_origin.get(Point());
	synfig::Vector orient=param_orient.get(Vector());

	static std::recursive_mutex freetype_mutex;

	if(needs_sync_)
		const_cast<Layer_Freetype*>(this)->sync();

	int error;
	Vector size(Layer_Freetype::param_size.get(synfig::Vector())*2);

	if(!context.accelerated_render(surface,quality,renddesc,cb))
		return false;

	if(is_disabled() || param_text.get(synfig::String()).empty())
		return true;

	// If there is no font loaded, just bail
	if(!face)
	{
		if(cb)cb->warning(std::string("Layer_Freetype:")+_("No face loaded, no text will be rendered."));
		return true;
	}

	String text(Layer_Freetype::param_text.get(synfig::String()));
	if(text=="@_FILENAME_@" && get_canvas() && !get_canvas()->get_file_name().empty())
	{
		text=basename(get_canvas()->get_file_name());
	}

	// Width and Height of a pixel
	Vector::value_type pw=renddesc.get_w()/(renddesc.get_br()[0]-renddesc.get_tl()[0]);
	Vector::value_type ph=renddesc.get_h()/(renddesc.get_br()[1]-renddesc.get_tl()[1]);

    // Calculate character width and height
	int w=std::abs(round_to_int(size[0]*pw));
	int h=std::abs(round_to_int(size[1]*ph));

    // If the font is the size of a pixel, don't bother rendering any text
	if(w<=1 || h<=1)
	{
		if(cb)cb->warning(std::string("Layer_Freetype:")+_("Text too small, no text will be rendered."));
		return true;
	}

	std::lock_guard<std::recursive_mutex> lock(freetype_mutex);

#define CHAR_RESOLUTION		(64)
	error = FT_Set_Char_Size(
		face,						// handle to face object
		(int)CHAR_RESOLUTION,	// char_width in 1/64th of points
		(int)CHAR_RESOLUTION,	// char_height in 1/64th of points
		round_to_int(std::abs(size[0]*pw*CHAR_RESOLUTION)),						// horizontal device resolution
		round_to_int(std::abs(size[1]*ph*CHAR_RESOLUTION)) );						// vertical device resolution

	// Here is where we can compensate for the
	// error in freetype's rendering engine.
	const Real xerror(std::abs(size[0]*pw)/(Real)face->size->metrics.x_ppem/1.13f/0.996);
	const Real yerror(std::abs(size[1]*ph)/(Real)face->size->metrics.y_ppem/1.13f/0.996);
	//synfig::info("xerror=%f, yerror=%f",xerror,yerror);
	const Real compress(Layer_Freetype::param_compress.get(Real())*xerror);
	const Real vcompress(Layer_Freetype::param_vcompress.get(Real())*yerror);

	if(error)
	{
		if(cb)cb->warning(std::string("Layer_Freetype:")+_("Unable to set face size.")+strprintf(" (err=%d)",error));
	}

	FT_GlyphSlot  slot = face->glyph;  // a small shortcut
	FT_UInt       glyph_index(0);
	FT_UInt       previous(0);
	int u,v;

	std::list<TextLine> lines;

	/*
 --	** -- CREATE GLYPHS -------------------------------------------------------
	*/

	mbstate_t ps;
	memset(&ps, 0, sizeof(ps));

	lines.push_front(TextLine());
	std::string::const_iterator iter;
	int bx=0;
	int by=0;

	for (iter=text.begin(); iter!=text.end(); ++iter)
	{
		int multiplier(1);
		if(*iter=='\n')
		{
			lines.push_front(TextLine());
			bx=0;
			by=0;
			previous=0;
			continue;
		}
		if(*iter=='\t')
		{
			multiplier=8;
			glyph_index = FT_Get_Char_Index( face, ' ' );
		}
		else
		{
			// read uft8 char
			unsigned int c = (unsigned char)*iter;
			unsigned int code = c;
			int bytes = 0;
			while ((c & 0x80) != 0) { c = (c << 1) & 0xff; bytes++; }
			bool bad_char = (bytes == 1);
			if (bytes > 1)
			{
				bytes--;
				code = c << (5*bytes - 1);
				while (bytes > 0) {
					iter++;
					bytes--;
					c = (unsigned char)*iter;
					if (iter >= text.end() || (c & 0xc0) != 0x80) { bad_char = true; break; }
					code |= (c & 0x3f) << (6 * bytes);
				}
			}

			if (bad_char)
			{
				synfig::warning("Layer_Freetype: multibyte: %s",
								_("Can't parse multibyte character.\n"));
				continue;
			}

			glyph_index = FT_Get_Char_Index( face, code );
		}

        // retrieve kerning distance and move pen position
		if ( FT_HAS_KERNING(face) && use_kerning && previous && glyph_index )
		{
			FT_Vector  delta;

			if(grid_fit)
				FT_Get_Kerning( face, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
			else
				FT_Get_Kerning( face, previous, glyph_index, FT_KERNING_UNFITTED, &delta );

			if(compress<1.0)
			{
				bx += round_to_int(delta.x*compress);
				by += round_to_int(delta.y*compress);
			}
			else
			{
				bx += delta.x;
				by += delta.y;
			}
        }

		Glyph curr_glyph;

        // store current pen position
        curr_glyph.pos.x = bx;
        curr_glyph.pos.y = by;

        // load glyph image into the slot. DO NOT RENDER IT !!
        if(grid_fit)
			error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT);
		else
			error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT|FT_LOAD_NO_HINTING );
        if (error) continue;  // ignore errors, jump to next glyph

        // extract glyph image and store it in our table
        error = FT_Get_Glyph( face->glyph, &curr_glyph.glyph );
        if (error) continue;  // ignore errors, jump to next glyph

        // record current glyph index
        previous = glyph_index;

		// Update the line width
		lines.front().width=bx+slot->advance.x;

		// increment pen position
		if(multiplier>1)
			bx += round_to_int(slot->advance.x*multiplier*compress)-bx%round_to_int(slot->advance.x*multiplier*compress);
		else
			bx += round_to_int(slot->advance.x*compress*multiplier);

		//bx += round_to_int(slot->advance.x*compress*multiplier);
		//by += round_to_int(slot->advance.y*compress);
		by += slot->advance.y*multiplier;

		lines.front().glyph_table.push_back(curr_glyph);

	}

	//Real	string_height;
	//string_height=(((lines.size()-1)*face->size->metrics.height+lines.back().actual_height()));

	//int string_height=face->size->metrics.ascender;
//#define METRICS_SCALE_ONE		(65536.0f)
#define METRICS_SCALE_ONE		((Real)(1<<16))

	Real line_height = vcompress*((Real)face->height*(((Real)face->size->metrics.y_scale/METRICS_SCALE_ONE)));
	Real text_height = (lines.size() - 1)*line_height + lines.back().actual_height();

	// This module sees to expect pixel height to be negative, as it
	// usually is.  But rendering to .bmp format causes ph to be
	// positive, which was causing text to be rendered upside down.
	//if (ph>0) line_height = -line_height;

	//synfig::info("string_height=%d",string_height);
	//synfig::info("line_height=%f",line_height);

	/*
 --	** -- RENDER THE GLYPHS ---------------------------------------------------
	*/

	Surface src_;
	Surface *src_surface;

	src_surface=surface;

	if(invert)
	{
		src_=*surface;
		Surface::alpha_pen pen(surface->begin(),get_amount(),get_blend_method());

		surface->fill(color,pen,src_.get_w(),src_.get_h());

		src_surface=&src_;
	}

	{
		int sign_y = ph >= 0.0 ? 1 : -1;
		Real offset_x = (origin[0]-renddesc.get_tl()[0])*pw*CHAR_RESOLUTION;
		Real offset_y = (origin[1]-renddesc.get_tl()[1])*ph*CHAR_RESOLUTION
				      - sign_y*text_height*(1.0 - orient[1]);

		std::list<TextLine>::iterator iter;
		int curr_line;
		for(curr_line=0,iter=lines.begin();iter!=lines.end();++iter,curr_line++)
		{
			bx=round_to_int(offset_x - orient[0]*iter->width);
			// I've no idea why 1.5, but it kind of works.  Otherwise,
			// rendering to .bmp (which renders from bottom to top, due to
			// the .bmp format describing the image from bottom to top,
			// renders text in the wrong place.
			by=round_to_int(offset_y + sign_y*curr_line*line_height);
			/*
			by=round_to_int((origin[1]-renddesc.get_tl()[1])*ph*CHAR_RESOLUTION +
							(1.0-orient[1])*string_height +
							(ph>0 ? line_height*(lines.size()-1-curr_line)-lines.back().actual_height(): -line_height*curr_line));
			*/

			//by=round_to_int(vcompress*((origin[1]-renddesc.get_tl()[1])*ph*64+(1.0-orient[1])*string_height-face->size->metrics.height*curr_line));
			//synfig::info("curr_line=%d, bx=%d, by=%d",curr_line,bx,by);

			std::vector<Glyph>::iterator iter2;
			for(iter2=iter->glyph_table.begin();iter2!=iter->glyph_table.end();++iter2)
			{
				FT_Glyph  image(iter2->glyph);
				FT_Vector pen;
				FT_BitmapGlyph  bit;

				pen.x = bx + iter2->pos.x;
				pen.y = by + iter2->pos.y;

				//synfig::info("GLYPH: line %d, pen.x=%d, pen,y=%d",curr_line,(pen.x+32)>>6,(pen.y+32)>>6);

				error = FT_Glyph_To_Bitmap( &image, FT_RENDER_MODE_NORMAL, nullptr/*&pen*/, 1 );
				if(error) { FT_Done_Glyph( image ); continue; }

				bit = (FT_BitmapGlyph)image;

				for(v=0;v<(int)bit->bitmap.rows;v++)
					for(u=0;u<(int)bit->bitmap.width;u++)
					{
						int x=u+((pen.x+32)>>6)+ bit->left;
						int y=((pen.y+32)>>6) + (bit->top - v) * sign_y;
						if(	y>=0 &&
							x>=0 &&
							y<surface->get_h() &&
							x<surface->get_w())
						{
							Real myamount=bit->bitmap.buffer[v*bit->bitmap.pitch+u]/Real(255.0);
							if(invert)
								myamount=1.0-myamount;
							(*surface)[y][x]=Color::blend(color,(*src_surface)[y][x],myamount*get_amount(),get_blend_method());
						}
					}

				FT_Done_Glyph( image );
			}
		}
	}

	return true;
}




synfig::Rect
Layer_Freetype::get_bounding_rect()const
{
	if(needs_sync_)
		const_cast<Layer_Freetype*>(this)->sync();
//	if(!is_disabled())
	return synfig::Rect::full_plane();
}
