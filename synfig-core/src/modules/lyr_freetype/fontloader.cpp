/* === S Y N F I G ========================================================= */
/*!	\file fontloader.cpp
**	\brief Implementation of the shared font-loading component
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

#include "fontloader.h"
#include "text_processing.h"

#include <algorithm>
#include <glibmm.h>

#if HAVE_HARFBUZZ
#include <hb-ft.h>
#endif

#include <synfig/filesystemnative.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/string_helper.h>

#endif

using namespace synfig;


/* === G L O B A L S ======================================================= */

#ifndef __APPLE__
static const std::vector<const char *> known_font_extensions = {".ttf", ".otf", ".ttc"};
#else
static const std::vector<const char *> known_font_extensions = {".ttf", ".otf", ".dfont", ".ttc"};
#endif

extern FT_Library ft_library;

FaceCache FontLoader::face_cache;

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
#endif

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
FontLoader::has_valid_font_extension(const std::string &filename) {
	std::string extension = filesystem::Path::filename_extension(filename);
	return std::find(known_font_extensions.begin(), known_font_extensions.end(), extension) != known_font_extensions.end();
}

/// Try to map a font family to a filename (without extension nor directory)
void
FontLoader::get_possible_font_filenames(synfig::String family, int style, int weight, std::vector<std::string>& list)
{
	strtolower(family);

	enum FontSuffixStyle {FONT_SUFFIX_NONE, FONT_SUFFIX_BI_BD, FONT_SUFFIX_BI_BD_IT, FONT_SUFFIX_BI_RI};
	enum FontClassification {FONT_SANS_SERIF, FONT_SERIF, FONT_MONOSPACED, FONT_SCRIPT};

	struct FontFileNameEntry {
		const char *alias;
		const char *prefix;
		const char *alternative_prefix;
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
				std::string filename = entry.prefix;
				filename += entry.get_suffix(style, weight);
				list.push_back(filename);

				filename = entry.prefix;
				filename += FontFileNameEntry::get_alternative_suffix(style, weight);
				list.push_back(filename);
			}
		}
	}
}

std::vector<std::string>
FontLoader::get_possible_font_directories(const std::string& canvas_path)
{
	std::vector<std::string> possible_font_directories = {std::string()};

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

std::vector<std::string>
FontLoader::get_possible_font_files(const std::string& newfont, const synfig::filesystem::Path& canvas_path)
{
	std::vector<std::string> possible_files;

	if (newfont.empty())
		return possible_files;

	std::vector<const char*> possible_font_extensions = {""};

	// if newfont doesn't have a known extension, try to append those extensions
	if (! has_valid_font_extension(newfont))
		possible_font_extensions.insert(possible_font_extensions.end(), known_font_extensions.begin(), known_font_extensions.end());

//	std::string canvas_path;
//	if (get_canvas())
//		canvas_path = get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR;

	std::vector<std::string> possible_font_directories = get_possible_font_directories(canvas_path.u8string());

	for (const std::string& directory : possible_font_directories) {
		for (const char *extension : possible_font_extensions) {
			std::string path = (directory + newfont + extension);
			if (FileSystemNative::instance()->is_file(path))
				possible_files.push_back(path);
		}
	}
	return possible_files;
}

#ifdef WITH_FONTCONFIG
std::string
FontLoader::fontconfig_get_filename(const std::string& font_fam, int style, int weight) {
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
#if FC_VERSION >= 21191
		case SYNFIG_TO_FC(SEMILIGHT);
#else
		case TEXT_WEIGHT_SEMILIGHT : fc_weight = FC_WEIGHT_LIGHT ; break;
#endif
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

std::string
FontLoader::canvas_font_dir(const synfig::filesystem::Path& canvas_path)
{
	std::string dir = canvas_path.u8string();
	if (!dir.empty() && dir.back() != ETL_DIRECTORY_SEPARATOR)
		dir += ETL_DIRECTORY_SEPARATOR;
	return dir;
}

FontLoader::LoadedFont
FontLoader::load_face(const std::string& newfont, const synfig::filesystem::Path& canvas_path)
{
	LoadedFont result;
	if (newfont.empty())
		return result;

	const std::string canvas_dir = canvas_font_dir(canvas_path);

	std::vector<std::string> filenames = get_possible_font_files(newfont, canvas_dir);
	if (filenames.empty())
		return result;

	int error = 0;
	for (const std::string& path : filenames) {
		const bool from_canvas = !canvas_dir.empty()
			&& path.compare(0, canvas_dir.size(), canvas_dir) == 0;

		filesystem::Path absolute_path = filesystem::absolute(path);

		if (FT_Face cached = face_cache.get(absolute_path)) {
			result.face = cached;
#if HAVE_HARFBUZZ
			result.font = FaceMetaData::get_from_face(cached).font;
#endif
			result.path_from_canvas = from_canvas;
			return result;
		}

		FT_Face new_face = nullptr;
		error = FT_New_Face(ft_library, path.c_str(), 0, &new_face);
		if (error)
			continue;

		face_cache.put(absolute_path, new_face);
#if HAVE_HARFBUZZ
		result.font = hb_ft_font_create(new_face, nullptr);
		FaceMetaData::add_to_face(new_face, path, result.font);
#else
		FaceMetaData::add_to_face(new_face, path);
#endif
		result.face = new_face;
		result.path_from_canvas = from_canvas;
		return result;
	}

	if (error)
		synfig::error(strprintf("FontLoader: %s (err=%d): %s",
			_("Unable to open font face."), error, newfont.c_str()));

	return result;
}

FontLoader::LoadedFont
FontLoader::load_font_once(const synfig::String& family, int style, int weight,
                           const synfig::filesystem::Path& canvas_path)
{
	LoadedFont result;
	if (family.empty())
		return result;

	const std::string canvas_dir = canvas_font_dir(canvas_path);

	auto lookup = [&](const std::string& cpath) {
		LoadedFont hit;
		FontMeta meta(family, style, weight);
		meta.canvas_path = cpath;
		if (FT_Face f = face_cache.get(meta)) {
			hit.face = f;
#if HAVE_HARFBUZZ
			hit.font = FaceMetaData::get_from_face(f).font;
#endif
			hit.path_from_canvas = !cpath.empty();
		}
		return hit;
	};

	// Canvas-relative fonts are keyed with the canvas dir; everything else
	// is keyed with an empty one, so both keys have to be probed.
	if (LoadedFont hit = lookup(canvas_dir))
		return hit;
	if (!canvas_dir.empty()) {
		if (LoadedFont hit = lookup(std::string()))
			return hit;
	}

	//resolution
	auto remember = [&](LoadedFont& r) -> LoadedFont& {
		FontMeta meta(family, style, weight);
		if (r.path_from_canvas)
			meta.canvas_path = canvas_dir;   // only canvas-relative hits are path-dependent
		face_cache.put(meta, r.face);
		return r;
	};

	if (has_valid_font_extension(family)) {
		if (LoadedFont r = load_face(family, canvas_path))
			return remember(r);
	}

#ifdef WITH_FONTCONFIG
	if (LoadedFont r = load_face(fontconfig_get_filename(family, style, weight), canvas_path))
		return remember(r);
#endif

	std::vector<std::string> filename_list;
	get_possible_font_filenames(family, style, weight, filename_list);
	for (const std::string& filename : filename_list) {
		if (LoadedFont r = load_face(filename, canvas_path))
			return remember(r);
	}

	if (LoadedFont r = load_face(family, canvas_path))
		return remember(r);

	return result;
}

FontLoader::LoadedFont
FontLoader::load_font(const synfig::String& family, int style, int weight,
                      const synfig::filesystem::Path& canvas_path)
{
	static const char* fallback = "sans serif";

	const struct { const char* fam; int style; int weight; } attempts[] = {
		{ nullptr,  style,              weight             },
		{ nullptr,  style,              TEXT_WEIGHT_NORMAL },
		{ nullptr,  TEXT_STYLE_NORMAL,  weight             },
		{ nullptr,  TEXT_STYLE_NORMAL,  TEXT_WEIGHT_NORMAL },
		{ fallback, style,              weight             },
		{ fallback, style,              TEXT_WEIGHT_NORMAL },
		{ fallback, TEXT_STYLE_NORMAL,  weight             },
		{ fallback, TEXT_STYLE_NORMAL,  TEXT_WEIGHT_NORMAL },
	};

	for (const auto& a : attempts) {
		const synfig::String fam = a.fam ? synfig::String(a.fam) : family;
		if (LoadedFont r = load_font_once(fam, a.style, a.weight, canvas_path))
			return r;
	}

	return LoadedFont();
}
