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

#include <pango/pangocairo.h>

#include "lyr_freetype.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/canvasfilenaming.h>
#include <synfig/cairo_renddesc.h>

#include <synfig/context.h>

//#ifdef __APPLE__
//#define USE_MAC_FT_FUNCS	(1)
//#endif

#ifdef USE_MAC_FT_FUNCS
	#include <CoreServices/CoreServices.h>
	#include FT_MAC_H
#endif

#endif

using namespace std;
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
SYNFIG_LAYER_SET_VERSION(Layer_Freetype,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_Freetype,"$Id$");

/* === C L A S S E S ======================================================= */

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

/* === P R O C E D U R E S ================================================= */

/*Glyph::~Glyph()
{
	if(glyph)FT_Done_Glyph(glyph);
}
*/
void
TextLine::clear_and_free()
{
	std::vector<Glyph>::iterator iter;
	for(iter=glyph_table.begin();iter!=glyph_table.end();++iter)
	{
		if(iter->glyph)FT_Done_Glyph(iter->glyph);
		iter->glyph=0;
	}
	glyph_table.clear();
}

/* === M E T H O D S ======================================================= */

Layer_Freetype::Layer_Freetype()
{
	face=0;

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
	if(face)
		FT_Done_Face(face);
}

void
Layer_Freetype::on_canvas_set()
{
	Layer_Composite::on_canvas_set();
	synfig::String family=param_family.get(synfig::String());
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

/*! The new_font() function try to render
**	text until it work by simplyfing font style(s).
** In last chance, render text as "sans serif" Normal
** font style.
*/
bool
Layer_Freetype::new_font_(const synfig::String &font_fam_, int style, int weight)
{
	synfig::String font_fam(font_fam_);

	if(new_face(font_fam_))
		return true;

	//start evil hack
	for(unsigned int i=0;i<font_fam.size();i++)font_fam[i]=tolower(font_fam[i]);
	//end evil hack

	if(font_fam=="arial black")
	{
#ifndef __APPLE__
		if(new_face("ariblk"))
			return true;
		else
#endif
		font_fam="sans serif";
	}

	if(font_fam=="sans serif" || font_fam=="arial")
	{
		String arial("arial");
		if(weight>TEXT_WEIGHT_NORMAL)
			arial+='b';
		if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
			arial+='i';
		else
			if(weight>TEXT_WEIGHT_NORMAL) arial+='d';

		if(new_face(arial))
			return true;
#ifdef __APPLE__
		if(new_face("Helvetica RO"))
			return true;
#endif
	}

	if(font_fam=="comic" || font_fam=="comic sans")
	{
		String filename("comic");
		if(weight>TEXT_WEIGHT_NORMAL)
			filename+='b';
		if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
			filename+='i';
		else if(weight>TEXT_WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}

	if(font_fam=="courier" || font_fam=="courier new")
	{
		String filename("cour");
		if(weight>TEXT_WEIGHT_NORMAL)
			filename+='b';
		if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
			filename+='i';
		else if(weight>TEXT_WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}

	if(font_fam=="serif" || font_fam=="times" || font_fam=="times new roman")
	{
		String filename("times");
		if(weight>TEXT_WEIGHT_NORMAL)
			filename+='b';
		if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
			filename+='i';
		else if(weight>TEXT_WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}

	if(font_fam=="trebuchet")
	{
		String filename("trebuc");
		if(weight>TEXT_WEIGHT_NORMAL)
			filename+='b';
		if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
		{
			filename+='i';
			if(weight<=TEXT_WEIGHT_NORMAL) filename+='t';
		}
		else if(weight>TEXT_WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}

	if(font_fam=="sans serif" || font_fam=="luxi sans")
	{
		{
			String luxi("luxis");
			if(weight>TEXT_WEIGHT_NORMAL)
				luxi+='b';
			else
				luxi+='r';
			if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
				luxi+='i';

			if(new_face(luxi))
				return true;
		}
		if(new_face("arial"))
			return true;
		if(new_face("Arial"))
			return true;
	}
	if(font_fam=="serif" || font_fam=="times" || font_fam=="times new roman" || font_fam=="luxi serif")
	{
		{
			String luxi("luxir");
			if(weight>TEXT_WEIGHT_NORMAL)
				luxi+='b';
			else
				luxi+='r';
			if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
				luxi+='i';

			if(new_face(luxi))
				return true;
		}
		if(new_face("Times New Roman"))
			return true;
		if(new_face("Times"))
			return true;
	}
	if(font_fam=="luxi")
	{
		{
			String luxi("luxim");
			if(weight>TEXT_WEIGHT_NORMAL)
				luxi+='b';
			else
				luxi+='r';
			if(style==TEXT_STYLE_ITALIC||style==TEXT_STYLE_OBLIQUE)
				luxi+='i';

			if(new_face(luxi))
				return true;
		}

		if(new_face("Times New Roman"))
			return true;
		if(new_face("Times"))
			return true;
	}

	return new_face(font_fam_) || new_face(font_fam);

	return false;
}

#ifdef USE_MAC_FT_FUNCS
void fss2path(char *path, FSSpec *fss)
{
  int l;             //fss->name contains name of last item in path
  for(l=0; l<(fss->name[0]); l++) path[l] = fss->name[l + 1];
  path[l] = 0;

  if(fss->parID != fsRtParID) //path is more than just a volume name
  {
    int i, len;
    CInfoPBRec pb;

    pb.dirInfo.ioNamePtr = fss->name;
    pb.dirInfo.ioVRefNum = fss->vRefNum;
    pb.dirInfo.ioDrParID = fss->parID;
    do
    {
      pb.dirInfo.ioFDirIndex = -1;  //get parent directory name
      pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrParID;
      if(PBGetCatInfoSync(&pb) != noErr) break;

      len = fss->name[0] + 1;
      for(i=l; i>=0;  i--) path[i + len] = path[i];
      for(i=1; i<len; i++) path[i - 1] = fss->name[i]; //add to start of path
      path[i - 1] = ':';
      l += len;
} while(pb.dirInfo.ioDrDirID != fsRtDirID); //while more directory levels
  }
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
	{
		FT_Done_Face(face);
		face=0;
	}

	if (newfont.empty())
		return false;

	std::vector<const char *> possible_font_extensions = {"", ".ttf", ".otf"};
#ifdef __APPLE__
	possible_font_extensions.push_back(".dfont");
#endif
	std::vector<std::string> possible_font_directories = {""};
	if (get_canvas())
		possible_font_directories.push_back( get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR );

#ifdef _WIN32
	possible_font_directories.push_back("C:\\WINDOWS\\FONTS\\");
#else

#ifdef __APPLE__
	possible_font_directories.push_back("~/Library/Fonts/");
	possible_font_directories.push_back("/Library/Fonts/");
#endif

	possible_font_directories.push_back("/usr/share/fonts/truetype/");
	possible_font_directories.push_back("/usr/share/fonts/opentype/");

#endif

	for (std::string directory : possible_font_directories) {
		for (const char *extension : possible_font_extensions) {
			std::string path = (directory + newfont + extension);
			error = FT_New_Face(ft_library, path.c_str(), face_index, &face);
			if (!error)
				break;
		}
		if (!error)
			break;
	}

#ifdef USE_MAC_FT_FUNCS
	if(error)
	{
		FSSpec fs_spec;
		error=FT_GetFile_From_Mac_Name(newfont.c_str(),&fs_spec,&face_index);
		if(!error)
		{
			char filename[512];
			fss2path(filename,&fs_spec);
			//FSSpecToNativePathName(fs_spec,filename,sizeof(filename)-1, 0);

			error=FT_New_Face(ft_library, filename, face_index,&face);
			//error=FT_New_Face_From_FSSpec(ft_library, &fs_spec, face_index,&face);
			synfig::info(__FILE__":%d: \"%s\" (%s) -- ft_error=%d",__LINE__,newfont.c_str(),filename,error);
		}
		else
		{
			synfig::info(__FILE__":%d: \"%s\" -- ft_error=%d",__LINE__,newfont.c_str(),error);
			// Unable to generate fs_spec
		}
	}
#endif

#ifdef WITH_FONTCONFIG
	if(error)
	{
		FcFontSet *fs;
		FcResult result;
		if( !FcInit() )
		{
			synfig::warning("Layer_Freetype: fontconfig: %s",_("unable to initialize"));
			error = 1;
		} else {
			FcPattern* pat = FcNameParse((FcChar8 *) newfont.c_str());
			FcConfigSubstitute(0, pat, FcMatchPattern);
			FcDefaultSubstitute(pat);
			FcPattern *match;
			fs = FcFontSetCreate();
			match = FcFontMatch(0, pat, &result);
			if (match)
				FcFontSetAdd(fs, match);
			if (pat)
				FcPatternDestroy(pat);
			if(fs && fs->nfont){
				FcChar8* file;
				if( FcPatternGetString (fs->fonts[0], FC_FILE, 0, &file) == FcResultMatch )
					error=FT_New_Face(ft_library,(const char*)file,face_index,&face);
				FcFontSetDestroy(fs);
			} else
				synfig::warning("Layer_Freetype: fontconfig: %s",_("empty font set"));
		}
	}
#endif

	if(error)
	{
		if (!newfont.empty())
			synfig::error(strprintf("Layer_Freetype: %s (err=%d)",_("Unable to open font face."),error));
		return false;
	}

	// ???
	font=newfont;

	needs_sync_=true;
	return true;
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
			if (color.get_a() == 0)
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
		.add_enum_value(TEXT_STYLE_NORMAL, "normal" ,_("Normal"))
		.add_enum_value(TEXT_STYLE_OBLIQUE, "oblique" ,_("Oblique"))
		.add_enum_value(TEXT_STYLE_ITALIC, "italic" ,_("Italic"))
	);

	ret.push_back(ParamDesc("weight")
		.set_local_name(_("Weight"))
		.set_hint("enum")
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
		.set_scalar(1)
	);

	ret.push_back(ParamDesc("orient")
		.set_local_name(_("Orientation"))
		.set_description(_("Text Orientation"))
		.set_invisible_duck()
	);

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Text Position"))
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

	const Color color(color_func(pos,0));

	if(!face)
		return context.get_color(pos);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
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
		if(cb)cb->warning(string("Layer_Freetype:")+_("No face loaded, no text will be rendered."));
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
	int w=abs(round_to_int(size[0]*pw));
	int h=abs(round_to_int(size[1]*ph));

    //int bx=(int)((origin[0]-renddesc.get_tl()[0])*pw*64+0.5);
    //int by=(int)((origin[1]-renddesc.get_tl()[1])*ph*64+0.5);
    int bx=0;
    int by=0;

    // If the font is the size of a pixel, don't bother rendering any text
	if(w<=1 || h<=1)
	{
		if(cb)cb->warning(string("Layer_Freetype:")+_("Text too small, no text will be rendered."));
		return true;
	}

	std::lock_guard<std::recursive_mutex> lock(freetype_mutex);

#define CHAR_RESOLUTION		(64)
	error = FT_Set_Char_Size(
		face,						// handle to face object
		(int)CHAR_RESOLUTION,	// char_width in 1/64th of points
		(int)CHAR_RESOLUTION,	// char_height in 1/64th of points
		round_to_int(abs(size[0]*pw*CHAR_RESOLUTION)),						// horizontal device resolution
		round_to_int(abs(size[1]*ph*CHAR_RESOLUTION)) );						// vertical device resolution

	// Here is where we can compensate for the
	// error in freetype's rendering engine.
	const Real xerror(abs(size[0]*pw)/(Real)face->size->metrics.x_ppem/1.13f/0.996);
	const Real yerror(abs(size[1]*ph)/(Real)face->size->metrics.y_ppem/1.13f/0.996);
	//synfig::info("xerror=%f, yerror=%f",xerror,yerror);
	const Real compress(Layer_Freetype::param_compress.get(Real())*xerror);
	const Real vcompress(Layer_Freetype::param_vcompress.get(Real())*yerror);

	if(error)
	{
		if(cb)cb->warning(string("Layer_Freetype:")+_("Unable to set face size.")+strprintf(" (err=%d)",error));
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
	string::const_iterator iter;
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
				FT_Get_Kerning( face, previous, glyph_index, ft_kerning_default, &delta );
			else
				FT_Get_Kerning( face, previous, glyph_index, ft_kerning_unfitted, &delta );

			if(compress<1.0f)
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

				error = FT_Glyph_To_Bitmap( &image, ft_render_mode_normal,0/*&pen*/, 1 );
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
							Real myamount=(Real)bit->bitmap.buffer[v*bit->bitmap.pitch+u]/255.0f;
							if(invert)
								myamount=1.0f-myamount;
							(*surface)[y][x]=Color::blend(color,(*src_surface)[y][x],myamount*get_amount(),get_blend_method());
						}
					}

				FT_Done_Glyph( image );
			}
			//iter->clear_and_free();
		}
	}

	return true;
}

////
bool
Layer_Freetype::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	int style=param_style.get(int());
	int weight=param_weight.get(int());
	synfig::Vector size=param_size.get(Vector());
	synfig::Real compress=param_compress.get(synfig::Real());
	synfig::Real vcompress=param_vcompress.get(synfig::Real());
	bool invert=param_invert.get(bool());
	Color color=param_color.get(Color());
	synfig::Point origin=param_origin.get(Point());
	synfig::Vector orient=param_orient.get(Vector());
	synfig::String font=param_font.get(synfig::String());
	synfig::String text=param_text.get(synfig::String());

	if(!is_solid_color())
	{
		// Initially render what's behind us
		if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
		{
			if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
			return false;
		}
	}

	RendDesc workdesc(renddesc);

	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, workdesc))
		return false;

	// New expanded workdesc values
	const int ww=workdesc.get_w();
	const int wh=workdesc.get_h();
	const double wtlx=workdesc.get_tl()[0];
	const double wtly=workdesc.get_tl()[1];
	const double wpw=workdesc.get_pw();
	const double wph=workdesc.get_ph();
	const double wsx=1/wpw;
	const double wsy=1/wph;
	const double wtx=(-wtlx+origin[0])*wsx;
	const double wty=(-wtly+origin[1])*wsy;

	// Cairo context
	cairo_surface_t* subimage = NULL;
	cairo_surface_t* inverted = NULL;
	subimage=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
	cairo_t* subcr=cairo_create(subimage);
	cairo_t* invertcr = NULL;
	if(invert)
	{
		inverted=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
		invertcr=cairo_create(inverted);
		cairo_set_source_rgba(invertcr, color.get_r(), color.get_g(), color.get_b(), color.get_a());
		cairo_paint_with_alpha(invertcr, get_amount());
	}

	// Pango
	PangoLayout *layout;
	PangoFontDescription *font_description;
	// Pango Font
	font_description = pango_font_description_new ();
	pango_font_description_set_family (font_description, font.c_str());
	pango_font_description_set_weight (font_description, PangoWeight(weight));
	pango_font_description_set_style (font_description, PangoStyle(style));
	// The size is scaled to match Software render size (remove the scale?)
	Real sizex=1.75*fabs(size[0])*fabs(wsx);
	Real sizey=1.75*fabs(size[1])*fabs(wsy);
	Real vscale=sizey/sizex;
	pango_font_description_set_absolute_size (font_description, sizex * PANGO_SCALE );

	//Pango Layout
	layout = pango_cairo_create_layout (subcr);

	pango_layout_set_font_description (layout, font_description);
	pango_layout_set_text (layout, text.c_str(), -1);
	if(orient[0]<0.4)
		pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
	else if(orient[0]>0.6)
		pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
	else
		pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);

	pango_layout_set_single_paragraph_mode(layout, false);

	// Calculate the logical and ink rectangles of the layout before add spacing
	PangoRectangle ink_layout, logical_layout;
	PangoRectangle ink_rect, logical_rect;
	pango_layout_get_pixel_extents(layout, &ink_layout, &logical_layout);

	// Spacing
	// Horizontal
	PangoAttrList* attrlist=pango_attr_list_new();
	Real hspace=compress>1.0?0.4*sizex*(compress-1.0):(compress<1.0)?0.5*sizex*(compress-1.0):0;
	PangoAttribute* spacing=pango_attr_letter_spacing_new(hspace*PANGO_SCALE);
	pango_attr_list_insert_before(attrlist, spacing);
	pango_layout_set_attributes(layout, attrlist);

	// Vertical
	int total_lines=pango_layout_get_line_count(layout);
	Real vspace_total=vcompress>1.0?0.4*logical_layout.height*(vcompress-1.0):(vcompress<1.0)?0.6*logical_layout.height*(vcompress-1.0):0;
	Real vspace=0;
	if(total_lines>1)
		vspace=vspace_total/(total_lines-1);
	pango_layout_set_spacing(layout, vspace*PANGO_SCALE);

	// Recalculate extents due to spacing changes
	pango_layout_get_pixel_extents(layout, &ink_layout, &logical_layout);

	// Render text
	cairo_save(subcr);
	cairo_set_source_rgba(subcr, color.get_r(), color.get_g(), color.get_b(), color.get_a());
	cairo_scale(subcr, 1.0, vscale);
	pango_cairo_update_layout(subcr, layout);
	cairo_move_to(subcr, wtx-logical_layout.width*orient[0], (wty-(logical_layout.height+vspace_total)*vscale*orient[1])/vscale);
	pango_cairo_show_layout(subcr, layout);

	// Debug ink and logical lines
	if(0)
	{
		pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);
		// Render logical and ink rectangles
		cairo_save(subcr);
		cairo_set_source_rgb(subcr, 0.0, 1.0, 0.0);
		cairo_set_line_width(subcr, 1.0);
		cairo_rectangle(subcr, wtx+ink_rect.x-0.5-logical_layout.width*orient[0],
						wty+ink_rect.y-0.5-(logical_layout.height+vspace_total)*orient[1],
						ink_rect.width,
						ink_rect.height);
		cairo_stroke(subcr);
		cairo_restore(subcr);

		cairo_save(subcr);
		cairo_set_line_width(subcr, 1.0);
		cairo_set_source_rgb(subcr, 0.0, 0.0, 1.0);
		cairo_rectangle(subcr, wtx+logical_rect.x-0.5-logical_layout.width*orient[0],
						wty+logical_rect.y-0.5-(logical_layout.height+vspace_total)*orient[1],
						logical_rect.width,
						logical_rect.height);
		cairo_stroke(subcr);
		cairo_move_to(subcr, wtx+2, wty);
		cairo_arc(subcr, wtx, wty, 2.0, 0, 2*3.141516);
		cairo_fill(subcr);
		cairo_restore(subcr);
	}
	cairo_restore(subcr);

	cairo_save(cr);
	// Render the text on the target surface with the proper operator
	if(invert)
	{
		cairo_set_source_surface(invertcr, subimage, 0,0);
		cairo_set_operator(invertcr, CAIRO_OPERATOR_DEST_OUT);
		cairo_paint_with_alpha(invertcr, get_amount());
	}
	// Need to scale down to user coordinates before pass to cr
	cairo_translate(cr, wtlx, wtly);
	cairo_scale(cr, wpw, wph);
	if(invert)
		cairo_set_source_surface(cr, inverted, 0, 0);
	else
		cairo_set_source_surface(cr, subimage, 0, 0);
	if(is_solid_color())
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint(cr);
	}
	else
	{
		cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	}
	cairo_restore(cr);

	// Destroy and return
	cairo_surface_destroy(subimage);
	cairo_destroy(subcr);
	if(invert)
	{
		cairo_surface_destroy(inverted);
		cairo_destroy(invertcr);
	}
	pango_attr_list_unref(attrlist);
	g_object_unref (layout);
	pango_font_description_free (font_description);
	return true;
}
////


synfig::Rect
Layer_Freetype::get_bounding_rect()const
{
	if(needs_sync_)
		const_cast<Layer_Freetype*>(this)->sync();
//	if(!is_disabled())
		return synfig::Rect::full_plane();
}
