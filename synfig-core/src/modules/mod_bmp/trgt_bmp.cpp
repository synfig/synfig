/* === S Y N F I G ========================================================= */
/*!	\file trgt_bmp.cpp
**	\brief Bitmap Target
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <glib/gstdio.h>
#include "trgt_bmp.h"
#include <synfig/general.h>
#include <synfig/localization.h>

#include <cstdio>
#include <algorithm>
#include <functional>
#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace etl;

/* === I N F O ============================================================= */

SYNFIG_TARGET_INIT(bmp);
SYNFIG_TARGET_SET_NAME(bmp,"bmp");
SYNFIG_TARGET_SET_EXT(bmp,"bmp");
SYNFIG_TARGET_SET_VERSION(bmp,"0.1");

/* === C L A S S E S & S T R U C T S ======================================= */
namespace synfig {

/*struct BITMAPFILEHEADER
{
	unsigned char	bfType[2];
	unsigned long	bfSize;
	unsigned short	bfReserved1;
	unsigned short	bfReserved2;
	unsigned long	bfOffsetBits;
};

struct BITMAPINFOHEADER
{
	unsigned long	biSize;
	long			biWidth;
	long			biHeight;
	unsigned short	biPlanes;
	unsigned short	biBitCount;
	unsigned long	biCompression;
	unsigned long	biSizeImage;
	long			biXPelsPerMeter;
	long			biYPelsPerMeter;
	unsigned long	biClrUsed;
	unsigned long	biClrImportant;
};*/

#pragma pack(push,1)
namespace BITMAP
{
	typedef unsigned char uint8_t;
	typedef unsigned short int uint16_t;
	typedef int int32_t;
	typedef unsigned int uint32_t;

	struct FILEHEADER
	{
		uint8_t		bfType[2];
		uint32_t	bfSize;
		uint16_t	bfReserved1;
		uint16_t	bfReserved2;
		uint32_t	bfOffsetBits;
	};

	struct INFOHEADER
	{
		uint32_t	biSize;
		int32_t		biWidth;
		int32_t		biHeight;
		uint16_t	biPlanes;
		uint16_t	biBitCount;
		uint32_t	biCompression;
		uint32_t	biSizeImage;
		int32_t		biXPelsPerMeter;
		int32_t		biYPelsPerMeter;
		uint32_t	biClrUsed;
		uint32_t	biClrImportant;
	};
}
#pragma pack(pop)


}
/* === M E T H O D S ======================================================= */

#ifdef WORDS_BIGENDIAN
inline long little_endian(const long &x)
{
	long ret;
	char *big_e=(char *)&ret;
	char *lit_e=(char *)&x;
	big_e[0]=lit_e[3];
	big_e[1]=lit_e[2];
	big_e[2]=lit_e[1];
	big_e[3]=lit_e[0];
	return ret;
}
inline short little_endian_short(const short &x)
{
	short ret;
	char *big_e=(char *)&ret;
	char *lit_e=(char *)&x;
	big_e[0]=lit_e[1];
	big_e[1]=lit_e[0];
	return ret;
}
#else
#define little_endian(x)	(x)
#define little_endian_short(x)	(x)
#endif

bmp::bmp(const char *Filename, const synfig::TargetParam& params):
	rowspan(),
	imagecount(),
	multi_image(false),
	file(NULL),
	filename(Filename),
	buffer(NULL),
	color_buffer(NULL),
	pf()
{
	set_alpha_mode(TARGET_ALPHA_MODE_FILL);
	sequence_separator=params.sequence_separator;
}

bmp::~bmp()
{
	if(file)
		fclose(file);
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
bmp::set_rend_desc(RendDesc *given_desc)
{
	pf=PF_BGR;

    // Flip the image upside down,
	// because bitmaps are upside down.
    given_desc->set_flags(0);
	Point tl=given_desc->get_tl();
	Point br=given_desc->get_br();
	Point::value_type tmp;
	tmp=tl[1];
	tl[1]=br[1];
	br[1]=tmp;
	given_desc->set_tl(tl);
	given_desc->set_br(br);

	desc=*given_desc;
	if(desc.get_frame_end()-desc.get_frame_start()>0)
	{
		multi_image=true;
		imagecount=desc.get_frame_start();
	}
	else
		multi_image=false;

    return true;
}

void
bmp::end_frame()
{
	if(file)
		fclose(file);
	delete [] color_buffer;
	color_buffer=0;
	file=NULL;
	imagecount++;
}

bool
bmp::start_frame(synfig::ProgressCallback *callback)
{
	int w=desc.get_w(),h=desc.get_h();

	rowspan=4*((w*(pixel_size(pf)*8)+31)/32);
	if(multi_image)
	{
		String newfilename(filename_sans_extension(filename) +
						   sequence_separator +
						   etl::strprintf("%04d",imagecount) +
						   filename_extension(filename));
		file=g_fopen(newfilename.c_str(),POPEN_BINARY_WRITE_TYPE);
		if(callback)callback->task(newfilename+_(" (animated)"));
	}
	else
	{
		file=g_fopen(filename.c_str(),POPEN_BINARY_WRITE_TYPE);
		if(callback)callback->task(filename);
	}

	if(!file)
	{
		if(callback)callback->error(_("Unable to open file"));
		else synfig::error(_("Unable to open file"));
		return false;
	}

	synfig::BITMAP::FILEHEADER fileheader;
	synfig::BITMAP::INFOHEADER infoheader;

	fileheader.bfType[0]='B';
	fileheader.bfType[1]='M';
	fileheader.bfSize=little_endian(sizeof(synfig::BITMAP::FILEHEADER)+sizeof(synfig::BITMAP::INFOHEADER)+rowspan*h);
	fileheader.bfReserved1=0;
	fileheader.bfReserved2=0;
	fileheader.bfOffsetBits=little_endian(sizeof(synfig::BITMAP::FILEHEADER)+sizeof(synfig::BITMAP::INFOHEADER));

	infoheader.biSize=little_endian(sizeof(synfig::BITMAP::INFOHEADER));
	infoheader.biWidth=little_endian(w);
	infoheader.biHeight=little_endian(h);
	infoheader.biPlanes=little_endian_short((short)1);
	infoheader.biBitCount=little_endian_short((short)(pixel_size(pf)*8));
	infoheader.biCompression=little_endian(0);
	infoheader.biSizeImage=little_endian(0);
	infoheader.biXPelsPerMeter=little_endian((int)rend_desc().get_x_res());
	infoheader.biYPelsPerMeter=little_endian((int)rend_desc().get_y_res()); // pels per meter...?
	infoheader.biClrUsed=little_endian(0);
	infoheader.biClrImportant=little_endian(0);

	//fprintf(file,"BM");

	//if (!fwrite(&fileheader.bfSize,sizeof(synfig::BITMAPFILEHEADER)-4,1,file))
	if (!fwrite(&fileheader, sizeof(synfig::BITMAP::FILEHEADER), 1, file))
	{
		if(callback)callback->error(_("Unable to write file header to file"));
		else synfig::error(_("Unable to write file header to file"));
		return false;
	}

	if (!fwrite(&infoheader, sizeof(synfig::BITMAP::INFOHEADER), 1, file))
	{
		if(callback)callback->error(_("Unable to write info header"));
		else synfig::error(_("Unable to write info header"));
		return false;
	}

	delete [] buffer;
	buffer=new unsigned char[rowspan];

	delete [] color_buffer;
	color_buffer=new Color[desc.get_w()];

	return true;
}

Color *
bmp::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
bmp::end_scanline()
{
	if(!file)
		return false;

	color_to_pixelformat(buffer, color_buffer, pf, 0, desc.get_w());

	if(!fwrite(buffer,1,rowspan,file))
		return false;

	return true;
}
