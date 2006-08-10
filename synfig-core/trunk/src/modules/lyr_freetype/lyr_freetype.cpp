/*! ========================================================================
** Synfig
** Template File
** $Id: lyr_freetype.cpp,v 1.5 2005/01/24 05:00:18 darco Exp $
**
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2006 Paul Wise
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


#endif

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define MAX_GLYPHS		2000

#define PANGO_STYLE_NORMAL (0)
#define PANGO_STYLE_OBLIQUE (1)
#define PANGO_STYLE_ITALIC (2)


#define WEIGHT_NORMAL (400)
#define WEIGHT_BOLD (700)

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(lyr_freetype);
SYNFIG_LAYER_SET_NAME(lyr_freetype,"text");
SYNFIG_LAYER_SET_LOCAL_NAME(lyr_freetype,_("Simple Text"));
SYNFIG_LAYER_SET_CATEGORY(lyr_freetype,_("Typography"));
SYNFIG_LAYER_SET_VERSION(lyr_freetype,"0.2");
SYNFIG_LAYER_SET_CVS_ID(lyr_freetype,"$Id: lyr_freetype.cpp,v 1.5 2005/01/24 05:00:18 darco Exp $");

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

lyr_freetype::lyr_freetype()
{
	face=0;
	
	size=Vector(0.25,0.25);
	text=_("Text Layer");
	color=Color::black();
	pos=Vector(0,0);
	orient=Vector(0.5,0.5);
	compress=1.0;
	vcompress=1.0;
	weight=WEIGHT_NORMAL;
	style=PANGO_STYLE_NORMAL;
	family="Sans Serif";
	use_kerning=true;
	grid_fit=false;
	old_version=false;
	set_blend_method(Color::BLEND_COMPOSITE);
	needs_sync_=true;
	
	new_font(family,style,weight);
	
	invert=false;
}

lyr_freetype::~lyr_freetype()
{
	if(face)
		FT_Done_Face(face);
}

void
lyr_freetype::new_font(const synfig::String &family, int style, int weight)
{		
	if(
		!new_font_(family,style,weight) &&
		!new_font_(family,style,WEIGHT_NORMAL) &&
		!new_font_(family,PANGO_STYLE_NORMAL,weight) &&
		!new_font_(family,PANGO_STYLE_NORMAL,WEIGHT_NORMAL) &&
		!new_font_("sans serif",style,weight) &&
		!new_font_("sans serif",style,WEIGHT_NORMAL) &&
		!new_font_("sans serif",PANGO_STYLE_NORMAL,weight)
	)
		new_font_("sans serif",PANGO_STYLE_NORMAL,WEIGHT_NORMAL);
}

bool
lyr_freetype::new_font_(const synfig::String &font_fam_, int style, int weight)
{
	synfig::String font_fam(font_fam_);

	if(new_face(font_fam_))
		return true;
	
	//start evil hack
	for(unsigned int i=0;i<font_fam.size();i++)font_fam[i]=tolower(font_fam[i]);
	//end evil hack

	if(font_fam=="arial black")
#ifndef __APPLE__
	if(new_face("ariblk"))
			return true;
		else
#endif
		font_fam="sans serif";
	
	if(font_fam=="sans serif" || font_fam=="arial")
	{
		String arial("arial");
		if(weight>WEIGHT_NORMAL)
			arial+='b';
		if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
			arial+='i';
		else
			if(weight>WEIGHT_NORMAL) arial+='d';

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
		if(weight>WEIGHT_NORMAL)
			filename+='b';
		if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
			filename+='i';
		else if(weight>WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}

	if(font_fam=="courier" || font_fam=="courier new")
	{
		String filename("cour");
		if(weight>WEIGHT_NORMAL)
			filename+='b';
		if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
			filename+='i';
		else if(weight>WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}

	if(font_fam=="serif" || font_fam=="times" || font_fam=="times new roman")
	{
		String filename("times");
		if(weight>WEIGHT_NORMAL)
			filename+='b';
		if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
			filename+='i';
		else if(weight>WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}
	
	if(font_fam=="trebuchet")
	{
		String filename("trebuc");
		if(weight>WEIGHT_NORMAL)
			filename+='b';
		if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
		{
			filename+='i';
			if(weight<=WEIGHT_NORMAL) filename+='t';
		}
		else if(weight>WEIGHT_NORMAL) filename+='d';

		if(new_face(filename))
			return true;
	}
		
	
	if(font_fam=="sans serif" || font_fam=="luxi sans")
	{
		{
			String luxi("luxis");
			if(weight>WEIGHT_NORMAL)
				luxi+='b';
			else
				luxi+='r';
			if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
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
			if(weight>WEIGHT_NORMAL)
				luxi+='b';
			else
				luxi+='r';
			if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
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
			if(weight>WEIGHT_NORMAL)
				luxi+='b';
			else
				luxi+='r';
			if(style==PANGO_STYLE_ITALIC||style==PANGO_STYLE_OBLIQUE)
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
lyr_freetype::new_face(const String &newfont)
{
	int error;
	FT_Long face_index=0;

	// If we are already loaded, don't bother reloading.
	if(face && font==newfont)
		return true;

	if(face)
	{
		FT_Done_Face(face);
		face=0;
	}

	error=FT_New_Face(ft_library,newfont.c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,(newfont+".ttf").c_str(),face_index,&face);

	if(get_canvas())
	{
		if(error)error=FT_New_Face(ft_library,(get_canvas()->get_file_path()+ETL_DIRECTORY_SEPERATOR+newfont).c_str(),face_index,&face);
		if(error)error=FT_New_Face(ft_library,(get_canvas()->get_file_path()+ETL_DIRECTORY_SEPERATOR+newfont+".ttf").c_str(),face_index,&face);
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
			synfig::warning("lyr_freetype: fontconfig: %s",_("unable to initialise"));
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
			if(fs){
				FcChar8* file;
				if( FcPatternGetString (fs->fonts[0], FC_FILE, 0, &file) == FcResultMatch )
					error=FT_New_Face(ft_library,(const char*)file,face_index,&face);
				FcFontSetDestroy(fs);
			} else
				synfig::warning("lyr_freetype: fontconfig: %s",_("empty font set"));
		}
	}
#endif

#ifdef WIN32
	if(error)error=FT_New_Face(ft_library,("C:\\WINDOWS\\FONTS\\"+newfont).c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("C:\\WINDOWS\\FONTS\\"+newfont+".ttf").c_str(),face_index,&face);
#else

#ifdef __APPLE__
	if(error)error=FT_New_Face(ft_library,("~/Library/Fonts/"+newfont).c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("~/Library/Fonts/"+newfont+".ttf").c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("~/Library/Fonts/"+newfont+".dfont").c_str(),face_index,&face);

	if(error)error=FT_New_Face(ft_library,("/Library/Fonts/"+newfont).c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("/Library/Fonts/"+newfont+".ttf").c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("/Library/Fonts/"+newfont+".dfont").c_str(),face_index,&face);
#endif

	if(error)error=FT_New_Face(ft_library,("/usr/X11R6/lib/X11/fonts/type1/"+newfont).c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("/usr/X11R6/lib/X11/fonts/type1/"+newfont+".ttf").c_str(),face_index,&face);

	if(error)error=FT_New_Face(ft_library,("/usr/share/fonts/truetype/"+newfont).c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("/usr/share/fonts/truetype/"+newfont+".ttf").c_str(),face_index,&face);

	if(error)error=FT_New_Face(ft_library,("/usr/X11R6/lib/X11/fonts/TTF/"+newfont).c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("/usr/X11R6/lib/X11/fonts/TTF/"+newfont+".ttf").c_str(),face_index,&face);

	if(error)error=FT_New_Face(ft_library,("/usr/X11R6/lib/X11/fonts/truetype/"+newfont).c_str(),face_index,&face);
	if(error)error=FT_New_Face(ft_library,("/usr/X11R6/lib/X11/fonts/truetype/"+newfont+".ttf").c_str(),face_index,&face);

#endif
	if(error)
	{
		//synfig::error(strprintf("lyr_freetype:%s (err=%d)",_("Unable to open face."),error));
		return false;
	}

	font=newfont;

	needs_sync_=true;
	return true;
}

bool
lyr_freetype::set_param(const String & param, const ValueBase &value)
{
	Mutex::Lock lock(mutex);
/*
	if(param=="font" && value.same_as(font))
	{
		new_font(etl::basename(value.get(font)),style,weight);
		family=etl::basename(value.get(font));
		return true;
	}
*/
	IMPORT_PLUS(family,new_font(family,style,weight));
	IMPORT_PLUS(weight,new_font(family,style,weight));
	IMPORT_PLUS(style,new_font(family,style,weight));
	IMPORT_PLUS(size, if(old_version){size/=2.0;} needs_sync_=true );
	IMPORT_PLUS(text,needs_sync_=true);
	IMPORT_PLUS(pos,needs_sync_=true);
	IMPORT(color);
	IMPORT(invert);
	IMPORT_PLUS(orient,needs_sync_=true);
	IMPORT_PLUS(compress,needs_sync_=true);
	IMPORT_PLUS(vcompress,needs_sync_=true);
	IMPORT_PLUS(use_kerning,needs_sync_=true);
	IMPORT_PLUS(grid_fit,needs_sync_=true);
	
	return Layer_Composite::set_param(param,value);
}

ValueBase
lyr_freetype::get_param(const String& param)const
{
	EXPORT(font);
	EXPORT(family);
	EXPORT(style);
	EXPORT(weight);
	EXPORT(size);
	EXPORT(text);
	EXPORT(color);
	EXPORT(pos);
	EXPORT(orient);
	EXPORT(compress);
	EXPORT(vcompress);
	EXPORT(use_kerning);
	EXPORT(grid_fit);
	EXPORT(invert);
	
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
lyr_freetype::get_param_vocab(void)const
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
		.add_enum_value(PANGO_STYLE_NORMAL, "normal" ,_("Normal"))
		.add_enum_value(PANGO_STYLE_OBLIQUE, "oblique" ,_("Oblique"))
		.add_enum_value(PANGO_STYLE_ITALIC, "italic" ,_("Italic"))
	);

	ret.push_back(ParamDesc("weight")
		.set_local_name(_("Weight"))
		.set_hint("enum")
		.add_enum_value(200, "ultralight" ,_("Ultralight"))
		.add_enum_value(300, "light" ,_("light"))
		.add_enum_value(400, "normal" ,_("Normal"))
		.add_enum_value(700, "bold" ,_("Bold"))
		.add_enum_value(800, "ultrabold" ,_("Ultrabold"))
		.add_enum_value(900, "heavy" ,_("Heavy"))
	);
	ret.push_back(ParamDesc("compress")
		.set_local_name(_("Hozontal Spacing"))
		.set_description(_("Describes how close glyphs are horizontally"))
	);

	ret.push_back(ParamDesc("vcompress")
		.set_local_name(_("Vertical Spacing"))
		.set_description(_("Describes how close lines of text are vertically"))
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the text"))
		.set_hint("size")
		.set_origin("pos")
		.set_scalar(1)
	);

	ret.push_back(ParamDesc("orient")
		.set_local_name(_("Orientation"))
		.set_description(_("Text Orientation"))
		.set_invisible_duck()
	);

	ret.push_back(ParamDesc("pos")
		.set_local_name(_("Position"))
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
		.set_description(_("Enables/Disables font kerning (If the font supports it)"))
	);

	ret.push_back(ParamDesc("grid_fit")
		.set_local_name(_("Sharpen Edges"))
		.set_description(_("Turn this off if you are going to be animating the text"))
	);
	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert"))
	);
	return ret;
}

void
lyr_freetype::sync()
{
	needs_sync_=false;
	
	
	
	
}

Color
lyr_freetype::get_color(Context context, const synfig::Point &pos)const
{
	if(needs_sync_)
		const_cast<lyr_freetype*>(this)->sync();
	
	if(!face)
		return context.get_color(pos);
	return context.get_color(pos);
}

bool
lyr_freetype::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	static synfig::RecMutex freetype_mutex;

	if(needs_sync_)
		const_cast<lyr_freetype*>(this)->sync();

	
	
	
	int error;
	Vector size(lyr_freetype::size*2);
	
	if(!context.accelerated_render(surface,quality,renddesc,cb))
		return false;
	
	if(is_disabled() || text.empty())
		return true;
	
	// If there is no font loaded, just bail
	if(!face)
	{
		if(cb)cb->warning(string("lyr_freetype:")+_("No face loaded, no text will be rendered."));
		return true;
	}

	String text(lyr_freetype::text);
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

    //int bx=(int)((pos[0]-renddesc.get_tl()[0])*pw*64+0.5);
    //int by=(int)((pos[1]-renddesc.get_tl()[1])*ph*64+0.5);
    int bx=0;
    int by=0;
	
    // If the font is the size of a pixel, don't bother rendering any text
	if(w<=1 || h<=1)
	{
		if(cb)cb->warning(string("lyr_freetype:")+_("Text too small, no text will be rendered."));
		return true;
	}

	synfig::RecMutex::Lock lock(freetype_mutex);

#define CHAR_RESOLUTION		(64)
	error = FT_Set_Char_Size(
		face,						// handle to face object           
		(int)CHAR_RESOLUTION,	// char_width in 1/64th of points 
		(int)CHAR_RESOLUTION,	// char_height in 1/64th of points 
		round_to_int(abs(size[0]*pw*CHAR_RESOLUTION)),						// horizontal device resolution    
		round_to_int(abs(size[1]*ph*CHAR_RESOLUTION)) );						// vertical device resolution      

	// Here is where we can compensate for the
	// error in freetype's rendering engine.
	const float xerror(abs(size[0]*pw)/(float)face->size->metrics.x_ppem/1.13f/0.996);
	const float yerror(abs(size[1]*ph)/(float)face->size->metrics.y_ppem/1.13f/0.996);
	//synfig::info("xerror=%f, yerror=%f",xerror,yerror);
	const float compress(lyr_freetype::compress*xerror);
	const float vcompress(lyr_freetype::vcompress*yerror);

	if(error)
	{
		if(cb)cb->warning(string("lyr_freetype:")+_("Unable to set face size.")+strprintf(" (err=%d)",error));
	}

	FT_GlyphSlot  slot = face->glyph;  // a small shortcut
	FT_UInt       glyph_index(0);
	FT_UInt       previous(0);
	int u,v;

	std::list<TextLine> lines;

	/*
 --	** -- CREATE GLYPHS -------------------------------------------------------
	*/

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
			glyph_index = FT_Get_Char_Index( face, *iter );

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
      

	//float	string_height;
	//string_height=(((lines.size()-1)*face->size->metrics.height+lines.back().actual_height()));

	//int string_height=face->size->metrics.ascender;
//#define METRICS_SCALE_ONE		(65536.0f)
#define METRICS_SCALE_ONE		((float)(1<<16))
	
	float line_height;
	line_height=vcompress*((float)face->height*(((float)face->size->metrics.y_scale/METRICS_SCALE_ONE)));

	int	string_height;
	string_height=round_to_int(((lines.size()-1)*line_height+lines.back().actual_height()));
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
	std::list<TextLine>::iterator iter;
	int curr_line;
	for(curr_line=0,iter=lines.begin();iter!=lines.end();++iter,curr_line++)
	{
		bx=round_to_int((pos[0]-renddesc.get_tl()[0])*pw*CHAR_RESOLUTION-orient[0]*iter->width);
		by=round_to_int((pos[1]-renddesc.get_tl()[1])*ph*CHAR_RESOLUTION+(1.0-orient[1])*string_height-line_height*curr_line);
		//by=round_to_int(vcompress*((pos[1]-renddesc.get_tl()[1])*ph*64+(1.0-orient[1])*string_height-face->size->metrics.height*curr_line));
		//synfig::info("curr_line=%d, bx=%d, by=%d",curr_line,bx,by);
		
		std::vector<Glyph>::iterator iter2;
		for(iter2=iter->glyph_table.begin();iter2!=iter->glyph_table.end();++iter2)
		{
			FT_Glyph  image(iter2->glyph);
			FT_Vector pen;
			FT_BitmapGlyph  bit;
				
			pen.x = bx + iter2->pos.x;
			pen.y = by + iter2->pos.y;
			
			//synfig::info("GLYPH: pen.x=%d, pen,y=%d",curr_line,(pen.x+32)>>6,(pen.y+32)>>6);
		
			error = FT_Glyph_To_Bitmap( &image, ft_render_mode_normal,0/*&pen*/, 1 );
			if(error) { FT_Done_Glyph( image ); continue; }
		
			bit = (FT_BitmapGlyph)image;

			for(v=0;v<bit->bitmap.rows;v++)
				for(u=0;u<bit->bitmap.width;u++)
				{
					int x=u+((pen.x+32)>>6)+ bit->left;
					int y=v+((pen.y+32)>>6)- bit->top;
					if(	y>=0 &&
						x>=0 &&
						y<surface->get_h() &&
						x<surface->get_w())
					{
						float myamount=(float)bit->bitmap.buffer[v*bit->bitmap.pitch+u]/255.0f;
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

synfig::Rect
lyr_freetype::get_bounding_rect()const
{
	if(needs_sync_)
		const_cast<lyr_freetype*>(this)->sync();
//	if(!is_disabled())
		return synfig::Rect::full_plane();
}
