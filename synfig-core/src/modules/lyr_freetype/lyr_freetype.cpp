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
#ifdef WITH_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#include "lyr_freetype.h"

#include <algorithm>
#include <glibmm.h>

#if HAVE_HARFBUZZ
#include <fribidi.h>
#include <hb-ft.h>
#endif

#include <synfig/canvasfilenaming.h>
#include <synfig/context.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/rendering/common/task/taskcontour.h>
#include <synfig/string_helper.h>

#endif

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

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

enum TextDirection{
	TEXT_DIRECTION_AUTO = 0,
	TEXT_DIRECTION_LTR = 1,
	TEXT_DIRECTION_RTL = 2,
};

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Freetype);
SYNFIG_LAYER_SET_NAME(Layer_Freetype,"text");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Freetype,N_("Text"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Freetype,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Freetype,"0.5");

#ifndef __APPLE__
static const std::vector<const char *> known_font_extensions = {".ttf", ".otf", ".ttc"};
#else
static const std::vector<const char *> known_font_extensions = {".ttf", ".otf", ".dfont", ".ttc"};
#endif

extern FT_Library ft_library;

/// NL/LF, VT, FF, CR, NEL, LS and PS
static const std::vector<uint32_t> line_endings{'\n', '\v', '\f', '\r', 0x0085, 0x2028, 0x2029};

/* === C L A S S E S ======================================================= */

#ifdef WITH_FONTCONFIG
// Allow proper finalization of FontConfig
struct FontConfigWrap {
	static FcConfig* instance() {
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

	explicit FontMeta(synfig::String family, int style=0, int weight=400)
		: family(std::move(family)), style(style), weight(weight)
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

struct FaceInfo {
	FT_Face face = nullptr;
#if HAVE_HARFBUZZ
	hb_font_t *font = nullptr;
#endif

	FaceInfo() = default;
	explicit FaceInfo(FT_Face ft_face)
		: face(ft_face)
	{
#if HAVE_HARFBUZZ
		font = hb_ft_font_create(face, nullptr);
#endif
	}
};

/// Cache font faces for speeding up the text layer rendering
class FaceCache {
	std::map<FontMeta, FaceInfo> cache;
	mutable std::mutex cache_mutex;
	FaceCache() = default; // Make constructor private to prevent instancing
public:
	FaceInfo get(const FontMeta &meta) const {
		std::lock_guard<std::mutex> lock(cache_mutex);
		auto iter = cache.find(meta);
		if (iter != cache.end())
			return iter->second;
		return FaceInfo();
	}

	void put(const FontMeta &meta, FaceInfo face) {
		std::lock_guard<std::mutex> lock(cache_mutex);
		cache[meta] = face;
	}

	bool has(const FontMeta &meta) const {
		std::lock_guard<std::mutex> lock(cache_mutex);
		auto iter = cache.find(meta);
		return iter != cache.end();
	}

	void clear() {
		std::lock_guard<std::mutex> lock(cache_mutex);
		for (const auto& item : cache) {
			FT_Done_Face(item.second.face);
#if HAVE_HARFBUZZ
			hb_font_destroy(item.second.font);
#endif
		}
		cache.clear();
	}

	static FaceCache& instance() {
		static FaceCache obj;
		return obj;
	}

	FaceCache(const FaceCache&) = delete; // Copy prohibited
	void operator=(const FaceCache&) = delete; // Assignment prohibited
	FaceCache& operator=(FaceCache&&) = delete; // Move assignment prohibited

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
				filename += FontFileNameEntry::get_alternative_suffix(style, weight);
				list.push_back(filename);
			}
		}
	}
}

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
	if (!has_valid_font_extension(family) || etl::is_absolute_path(family))
		return;

	int style=param_style.get(int());
	int weight=param_weight.get(int());
	new_font(family,style,weight);
}

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

	{
		FaceInfo face_info = face_cache.get(meta);
		FT_Face tmp_face = face_info.face;
		if (tmp_face) {
			if (face != tmp_face)
				need_sync |= SYNC_FONT;
			face = tmp_face;
	#if HAVE_HARFBUZZ
			font = face_info.font;
	#endif
			return true;
		}
	}

	auto cache_face = [&](FT_Face face) {
		if (!font_path_from_canvas)
			meta.canvas_path.clear();
		FaceInfo face_info(face);
		face_cache.put(meta, FaceInfo(face));
#if HAVE_HARFBUZZ
		font = face_info.font;
#endif
	};

	if (has_valid_font_extension(font_fam_))
		if (new_face(font_fam_)) {
			cache_face(face);
			return true;
		}

#ifdef WITH_FONTCONFIG
	if (new_face(fontconfig_get_filename(font_fam_, style, weight))) {
		cache_face(face);
		return true;
	}
#endif

	std::vector<std::string> filename_list;
	get_possible_font_filenames(font_fam_, style, weight, filename_list);

	for (std::string& filename : filename_list) {
		if (new_face(filename)) {
			cache_face(face);
			return true;
		}
	}
	if (new_face(font_fam_)) {
		cache_face(face);
		return true;
	}

	return false;
}

#ifdef WITH_FONTCONFIG

static std::string fontconfig_get_filename(const std::string& font_fam, int style, int weight) {
	std::string filename;
	FcConfig* fc = FontConfigWrap::instance();
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

	for (const std::string& directory : possible_font_directories) {
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
		synfig::error(strprintf("Layer_Freetype: %s (err=%d): %s",_("Unable to open font face."),error,newfont.c_str()));
		return false;
	}

	// ???
	font=newfont;

	need_sync |= SYNC_FONT;
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
	if (userdir.empty()) {
		synfig::error(strprintf("Layer_Freetype: %s", _("Cannot retrieve user home folder")));
	} else {
		possible_font_directories.push_back(userdir+"/Library/Fonts/");
	}
	possible_font_directories.push_back("/Library/Fonts/");
#endif

	possible_font_directories.push_back("/usr/share/fonts/truetype/");
	possible_font_directories.push_back("/usr/share/fonts/opentype/");

#endif

	return possible_font_directories;
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
		auto text=basename(get_canvas()->get_file_name());
		lines = fetch_text_lines(text, direction);
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

			hb_direction_t direction = HB_DIRECTION_LTR; // character order already fixed by FriBiDi
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
				convert_outline_to_contours(outline_glyph, glyph.outline);
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
				shift_contour_chunks(chunks, offset);
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
		shift_contour_chunks(visual_line, offset);
		add(visual_line);
	}
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
Layer_Freetype::get_color(Context context, const Point &pos)const
{
	return Layer_Shape::get_color(context, pos);
}

Layer::Handle
Layer_Freetype::hit_check(Context context, const Point &point) const
{
	sync();

	Point new_point = world_to_contour(point);
	return Layer_Shape::hit_check(context, new_point);
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

	lines = fetch_text_lines(param_text.get(std::string()), param_direction.get(0));

	need_sync |= SYNC_TEXT;
}

static std::vector<uint32_t>
utf8_to_utf32(const std::string& text)
{
	std::vector<uint32_t> unicode;
#if HAVE_HARFBUZZ
	unicode.resize(text.size()+1);
	FriBidiStrIndex unicode_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, text.c_str(), text.size(), unicode.data());
	unicode.resize(unicode_len);
#else
	for (auto iter = text.cbegin(); iter != text.cend(); ++iter) {
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
				++iter;
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

		unicode.push_back(code);
	}
#endif
	return unicode;
}

std::vector<Layer_Freetype::TextLine>
Layer_Freetype::fetch_text_lines(const std::string& text, int direction)
{
	std::vector<TextLine> new_lines;

	if (text.empty())
		return new_lines;

	std::vector<uint32_t> unicode;

	{
		std::string parsed_text = text;
		// 0. Do some pre-parsing still in UTF-8
		{
			// 0.1 \r\n -> \n
			// 0.2 \t -> 8 blank spaces
			auto pos = parsed_text.find_first_of("\r\t");
			while (pos != std::string::npos) {
				if (parsed_text[pos] == '\t') {
					const char *tab = "        ";
					const int tab_length = 8;
					parsed_text.replace(pos, 1, tab);
					pos += tab_length;
				} else if (/*parsed_text[pos] == '\r' &&*/ pos+1 != std::string::npos && parsed_text[pos+1] == '\n') {
					parsed_text.erase(pos, 1);
				}
				pos = parsed_text.find_first_of("\r\t", pos);
			}
		}

		// 1. Convert to unicode codepoints
		unicode = utf8_to_utf32(parsed_text);
	}

	// 2. Split into lines
	std::vector<std::vector<uint32_t>> base_lines;
	{
		auto it = unicode.begin();
		while (true) {
			auto new_it = std::find_first_of(it, unicode.end(), line_endings.begin(), line_endings.end());
			std::vector<uint32_t> line{it, new_it};
			base_lines.push_back(line);

			if (new_it == unicode.end())
				break;
			it = new_it+1;
		}
	}

	// 3. Handle BiDirectional text
#if HAVE_HARFBUZZ
	for (auto& line : base_lines) {
		FriBidiParType base_dir = direction == TEXT_DIRECTION_AUTO ? FRIBIDI_TYPE_ON : direction == TEXT_DIRECTION_LTR ? FRIBIDI_TYPE_LTR : FRIBIDI_TYPE_RTL;
		// FriBiDi + HarfBuzz: don't use FriBiDi simple shaper, just get the BiDi info / character reordering
		size_t line_size = line.size();
		std::vector<FriBidiCharType> bidi_types(line_size);
		fribidi_get_bidi_types(line.data(), line_size, bidi_types.data());
		std::vector<FriBidiLevel> bidi_levels(line_size);
#if FRIBIDI_MAJOR_VERSION >= 1
		std::vector<FriBidiBracketType> bracket_types(line_size);
		FriBidiLevel fribidi_result = fribidi_get_par_embedding_levels_ex(bidi_types.data(), bracket_types.data(), line_size,
															  &base_dir, bidi_levels.data());
#else
		FriBidiLevel fribidi_result = fribidi_get_par_embedding_levels(bidi_types.data(), line_size,
																	   &base_dir, bidi_levels.data());
#endif
		if (fribidi_result == 0) {
			synfig::error("Layer_Freetype: %s", _("error running FriBiDi (getting embedding levels)"));
			return new_lines;
		}

		fribidi_result = fribidi_reorder_line(FRIBIDI_FLAGS_DEFAULT|FRIBIDI_FLAGS_ARABIC, bidi_types.data(), line_size, 0, base_dir,
							 bidi_levels.data(), line.data(), nullptr);
		if (fribidi_result == 0) {
			synfig::error("Layer_Freetype: %s", _("Layer_FreeType: error running FriBiDi (reordering line)"));
			return new_lines;
		}
	}
#endif

	// 4. Split text into lines (and text spans according to their script if we know them)
#if not HAVE_HARFBUZZ
	for (const auto& line : base_lines) {
		TextLine current_line;

		for (uint32_t codepoint : line) {
			if (!current_line.empty()) {
				current_line.back().codepoints.push_back(codepoint);
			} else {
				current_line.push_back(TextSpan{{codepoint}});
			}
		}
		new_lines.push_back(current_line);
	}
#else
	hb_unicode_funcs_t* ufuncs = hb_unicode_funcs_get_default();
	for (const auto& line : base_lines) {
		TextLine current_line;
		hb_script_t current_script = HB_SCRIPT_INVALID;

		for (uint32_t codepoint : line) {
			hb_script_t script = hb_unicode_script(ufuncs, codepoint);

			if (!current_line.empty() && (script == current_script || script == HB_SCRIPT_INHERITED)) {
				current_line.back().codepoints.push_back(codepoint);
			} else {
				current_line.push_back(TextSpan{
										   {codepoint},
										   script
									   });
				current_script = script;
			}
		}
		new_lines.push_back(current_line);
	}
#endif
	return new_lines;
}

void
Layer_Freetype::convert_outline_to_contours(const FT_OutlineGlyphRec* glyph, rendering::Contour::ChunkList& chunks)
{
	auto get_vector = [] (const FT_OutlineGlyphRec* glyph, short index) -> Vector {
		const FT_Vector& ft_v = glyph->outline.points[index];
		return Vector(ft_v.x, ft_v.y);
	};

	chunks.clear();

	if (!glyph) {
		synfig::error(strprintf("Layer_Freetype: %s", _("Outline Glyph is null!")));
		return;
	}

	if (glyph->outline.n_contours == 0) {
		// No contours? OK, it can be a whitespace
		return;
	}

	short p = 0;
	rendering::Contour contour;
	for (int nc = 0; nc < glyph->outline.n_contours; nc++) {
		if (glyph->outline.n_points == 0)
			continue;

		const short first_p = p;
		const short max_p = std::min(glyph->outline.contours[nc], glyph->outline.n_points);
		short last_p = max_p;

		{
			const Vector v = get_vector(glyph, p);
			const char tag = FT_CURVE_TAG(glyph->outline.tags[p]);

			switch (tag) {
			case FT_CURVE_TAG_ON:
				contour.move_to(v);
				break;
			case FT_CURVE_TAG_CONIC: {
				char last_tag = FT_CURVE_TAG(glyph->outline.tags[max_p]);
				Vector last_v = get_vector(glyph, max_p);
				switch (last_tag) {
				case FT_CURVE_TAG_ON:
					contour.move_to(last_v);
					last_p--;
					break;
				case FT_CURVE_TAG_CONIC:
					last_v = (v + last_v)/2;
					contour.move_to(last_v);
					break;
				case FT_CURVE_TAG_CUBIC:
					synfig::error("Layer_Freetype: %s", _("the glyph outline contour cannot end with cubic bezier control point"));
					continue;
				default:
					synfig::error("Layer_Freetype: %s", _("unknown previous tag for the glyph outline contour"));
					continue;
				}
				break;
			}
			case FT_CURVE_TAG_CUBIC:
				synfig::error("Layer_Freetype: %s", _("the glyph outline contour cannot start with cubic bezier control point"));
				continue;
			default:
				synfig::error("Layer_Freetype: %s", _("unknown tag for the glyph outline contour"));
				continue;
			}
		}

		while (p <= last_p) {
			short next_p = p + 1;
			if (next_p > max_p)
				next_p = first_p;
			short next2_p = next_p + 1;
			if (next2_p > max_p)
				next2_p = first_p;

			const Vector v = get_vector(glyph, p);
			const Vector next_v = get_vector(glyph, next_p);
			const Vector next2_v = get_vector(glyph, next2_p);

			const char tag = FT_CURVE_TAG(glyph->outline.tags[p]);
			const char next_tag = FT_CURVE_TAG(glyph->outline.tags[next_p]);
			const char next2_tag = FT_CURVE_TAG(glyph->outline.tags[next2_p]);

			if (tag == FT_CURVE_TAG_ON && next_tag == FT_CURVE_TAG_ON) {
				contour.line_to(next_v);
				p += 1;
			} else if (tag == FT_CURVE_TAG_ON && next_tag == FT_CURVE_TAG_CONIC && next2_tag == FT_CURVE_TAG_ON) {
				contour.conic_to(next2_v, next_v);
				p += 2;
			} else if (tag == FT_CURVE_TAG_ON && next_tag == FT_CURVE_TAG_CONIC && next2_tag == FT_CURVE_TAG_CONIC) {
				Vector target_v = (next_v + next2_v)/2;
				contour.conic_to(target_v, next_v);
				p += 2;
			} else if (tag == FT_CURVE_TAG_ON && next_tag == FT_CURVE_TAG_CUBIC && next2_tag == FT_CURVE_TAG_CUBIC) {
				short next3_p = next2_p + 1;
				if (next3_p > max_p)
					next3_p = first_p;

				const char next3_tag = FT_CURVE_TAG(glyph->outline.tags[next3_p]);
				if (next3_tag == FT_CURVE_TAG_ON) {
					const Vector next3_v = get_vector(glyph, next3_p);
					contour.cubic_to(next3_v, next_v, next2_v);
				}
				p += 3;
			} else if (tag == FT_CURVE_TAG_CONIC && next_tag == FT_CURVE_TAG_ON) {
				contour.conic_to(next_v, v);
				p += 1;
			} else if (tag == FT_CURVE_TAG_CONIC && next_tag == FT_CURVE_TAG_CONIC) {
				Vector middle = (v + next_v)/2;
				contour.conic_to(middle, v);
				p += 1;
			} else { // cuBIC?!
				synfig::warning("Layer_Freetype: %s", _("strange glyph vertex component... Aborting"));
				break;
			}
		}

		contour.close();

		chunks = contour.get_chunks();
	}
}

void
Layer_Freetype::shift_contour_chunks(synfig::rendering::Contour::ChunkList& chunks, const Vector& offset)
{
	for (auto& chunk : chunks) {
		chunk.p1 += offset;
		chunk.pp0 += offset;
		chunk.pp1 += offset;
	}
}

Point
Layer_Freetype::world_to_contour(const synfig::Point &p) const
{
	if (!face)
		return p;
	Vector size = param_size.get(Vector()) * 2;

	Matrix matrix = Matrix().set_translate(param_origin.get(Vector()))
					* Matrix().set_scale(size/(face->units_per_EM))
					* Matrix().set_translate(param_origin.get(Vector()));

	return matrix.get_inverted().get_transformed(p);
}

Point Layer_Freetype::contour_to_world(const synfig::Point &p) const
{
	if (!face)
		return p;
	Vector size = param_size.get(Vector()) * 2;

	Matrix matrix = Matrix().set_translate(param_origin.get(Vector()))
					* Matrix().set_scale(size/(face->units_per_EM))
					* Matrix().set_translate(param_origin.get(Vector()));

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
