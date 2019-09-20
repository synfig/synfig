/* === S Y N F I G ========================================================= */
/*!	\file mptr_bmp.cpp
**	\brief bmp Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_bmp.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/surface.h>

#include <algorithm>
#include <functional>
#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(bmp_mptr);
SYNFIG_IMPORTER_SET_NAME(bmp_mptr,"bmp");
SYNFIG_IMPORTER_SET_EXT(bmp_mptr,"bmp");
SYNFIG_IMPORTER_SET_VERSION(bmp_mptr,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(bmp_mptr,"$Id$");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(bmp_mptr, true);

/* === M E T H O D S ======================================================= */
namespace synfig {

#pragma pack(push, 1)

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


bool
bmp_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time /*time*/, synfig::ProgressCallback *cb)
{
	FileSystem::ReadStream::Handle stream = identifier.get_read_stream();
	if(!stream)
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+strprintf(_("Unable to open %s"),identifier.filename.c_str()));
		else synfig::error("bmp_mptr::GetFrame(): "+strprintf(_("Unable to open %s"),identifier.filename.c_str()));
		return false;
	}

	synfig::BITMAP::FILEHEADER fileheader;
	synfig::BITMAP::INFOHEADER infoheader;

	if (!stream->read_variable(fileheader.bfType)
	 || fileheader.bfType[0] != 'B'
	 || fileheader.bfType[1] != 'M')
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+strprintf(_("%s is not in BMP format"),identifier.filename.c_str()));
		else synfig::error("bmp_mptr::GetFrame(): "+strprintf(_("%s is not in BMP format"),identifier.filename.c_str()));
		return false;
	}

	if(!stream->read_whole_block(&fileheader.bfSize, sizeof(synfig::BITMAP::FILEHEADER)-2))
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Failure while reading BITMAP::FILEHEADER from %s"),identifier.filename.c_str()));
		if(cb)cb->error(str);
		else synfig::error(str);
		return false;
	}

	if(!stream->read_whole_block(&infoheader, sizeof(synfig::BITMAP::INFOHEADER)))
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Failure while reading BITMAP::INFOHEADER from %s"),identifier.filename.c_str()));
		if(cb)cb->error(str);
		else synfig::error(str);
		return false;
	}

	int offset=little_endian(fileheader.bfOffsetBits);

	if(offset!=sizeof(synfig::BITMAP::FILEHEADER)+sizeof(synfig::BITMAP::INFOHEADER))
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Bad BITMAP::FILEHEADER in %s. (bfOffsetBits=%d, should be %zu)"),identifier.filename.c_str(),offset,sizeof(synfig::BITMAP::FILEHEADER)+sizeof(synfig::BITMAP::INFOHEADER)));
		if(cb)cb->error(str);
		else synfig::error(str);
		return false;
	}

	if(little_endian(infoheader.biSize)!=sizeof(synfig::BITMAP::INFOHEADER))
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Bad BITMAP::INFOHEADER in %s. (biSize=%d, should be %zu)"),identifier.filename.c_str(),little_endian(infoheader.biSize),sizeof(synfig::BITMAP::INFOHEADER)));
		if(cb)cb->error(str);
		else synfig::error(str);
		return false;
	}

	int w,h,bit_count;

	w=little_endian(infoheader.biWidth);
	h=little_endian(infoheader.biHeight);
	bit_count=little_endian_short(infoheader.biBitCount);

	synfig::warning("w:%d\n",w);
	synfig::warning("h:%d\n",h);
	synfig::warning("bit_count:%d\n",bit_count);

	if(little_endian(infoheader.biCompression))
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+string(_("Reading compressed bitmaps is not supported")));
		else synfig::error("bmp_mptr::GetFrame(): "+string(_("Reading compressed bitmaps is not supported")));
		return false;
	}

	if(bit_count!=24 && bit_count!=32)
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+strprintf(_("Unsupported bit depth (bit_count=%d, should be 24 or 32)"),bit_count));
		else synfig::error("bmp_mptr::GetFrame(): "+strprintf(_("Unsupported bit depth (bit_count=%d, should be 24 or 32)"),bit_count));
		return false;
	}

	int x;
	int y;
	surface.set_wh(w,h);
	for(y=0;y<surface.get_h();y++)
		for(x=0;x<surface.get_w();x++)
		{
//			float b=(float)(unsigned char)stream->getc()*(1.0/255.0);
//			float g=(float)(unsigned char)stream->getc()*(1.0/255.0);
//			float r=(float)(unsigned char)stream->getc()*(1.0/255.0);
			float b=gamma().b_U8_to_F32((unsigned char)stream->get());
			float g=gamma().g_U8_to_F32((unsigned char)stream->get());
			float r=gamma().r_U8_to_F32((unsigned char)stream->get());

			surface[h-y-1][x]=Color(
				r,
				g,
				b,
				1.0
			);
			if(bit_count==32)
				stream->get();
		}


	return true;
}

