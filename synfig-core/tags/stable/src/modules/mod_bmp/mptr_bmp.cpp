/*! ========================================================================
** Sinfg
** bmp Target Module
** $Id: mptr_bmp.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SINFG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_bmp.h"
#include <sinfg/general.h>
#include <sinfg/surface.h>

#include <algorithm>
#include <functional>
#endif

/* === U S I N G =========================================================== */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_IMPORTER_INIT(bmp_mptr);
SINFG_IMPORTER_SET_NAME(bmp_mptr,"bmp_mptr");
SINFG_IMPORTER_SET_EXT(bmp_mptr,"bmp");
SINFG_IMPORTER_SET_VERSION(bmp_mptr,"0.1");
SINFG_IMPORTER_SET_CVS_ID(bmp_mptr,"$Id: mptr_bmp.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === M E T H O D S ======================================================= */

struct BITMAPFILEHEADER
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
};

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






bmp_mptr::bmp_mptr(const char *file)
{
	filename=file;
}

bmp_mptr::~bmp_mptr()
{
}

bool
bmp_mptr::get_frame(sinfg::Surface &surface,Time, sinfg::ProgressCallback *cb)
{
	FILE *file=fopen(filename.c_str(),"rb");
	if(!file)
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+strprintf(_("Unable to open %s"),filename.c_str())); 
		else sinfg::error("bmp_mptr::GetFrame(): "+strprintf(_("Unable to open %s"),filename.c_str()));
		return false;
	}

	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;
	char b_char=fgetc(file);
	char m_char=fgetc(file);
	
	if(b_char!='B' || m_char!='M')
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+strprintf(_("%s is not in BMP format"),filename.c_str())); 
		else sinfg::error("bmp_mptr::GetFrame(): "+strprintf(_("%s is not in BMP format"),filename.c_str()));
		return false;
	}
		
	if(fread(&fileheader.bfSize, 1, sizeof(BITMAPFILEHEADER)-4, file)!=sizeof(BITMAPFILEHEADER)-4)
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Failure while reading BITMAPFILEHEADER from %s"),filename.c_str())); 
		if(cb)cb->error(str); 
		else sinfg::error(str);
		return false;
	}
		
	if(fread(&infoheader, 1, sizeof(BITMAPINFOHEADER), file)!=sizeof(BITMAPINFOHEADER))
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Failure while reading BITMAPINFOHEADER from %s"),filename.c_str())); 
		if(cb)cb->error(str); 
		else sinfg::error(str);
		return false;
	}
	
	int offset=little_endian(fileheader.bfOffsetBits);
	
	if(offset!=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)-2)	
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Bad BITMAPFILEHEADER in %s. (bfOffsetBits=%d, should be %d)"),filename.c_str(),offset,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)-2)); 
		if(cb)cb->error(str); 
		else sinfg::error(str);
		return false;
	}

	if(little_endian(infoheader.biSize)!=little_endian(40))
	{
		String str("bmp_mptr::get_frame(): "+strprintf(_("Bad BITMAPINFOHEADER in %s. (biSize=%d, should be 40)"),filename.c_str(),little_endian(infoheader.biSize))); 
		if(cb)cb->error(str); 
		else sinfg::error(str);
		return false;
	}
	
	int w,h,bit_count;

	w=little_endian(infoheader.biWidth);
	h=little_endian(infoheader.biHeight);
	bit_count=little_endian_short(infoheader.biBitCount);
	
	sinfg::warning("w:%d\n",w);
	sinfg::warning("h:%d\n",h);
	sinfg::warning("bit_count:%d\n",bit_count);
	
	if(little_endian(infoheader.biCompression))
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+string(_("Reading compressed bitmaps is not supported"))); 
		else sinfg::error("bmp_mptr::GetFrame(): "+string(_("Reading compressed bitmaps is not supported")));
		return false;
	}

	if(bit_count!=24 && bit_count!=32)
	{
		if(cb)cb->error("bmp_mptr::GetFrame(): "+strprintf(_("Unsupported bit depth (bit_count=%d, should be 24 or 32)"),bit_count)); 
		else sinfg::error("bmp_mptr::GetFrame(): "+strprintf(_("Unsupported bit depth (bit_count=%d, should be 24 or 32)"),bit_count));
		return false;
	}
	
	int x;
	int y;
	surface.set_wh(w,h);
	for(y=0;y<surface.get_h();y++)
		for(x=0;x<surface.get_w();x++)
		{
//			float b=(float)(unsigned char)fgetc(file)*(1.0/255.0);
//			float g=(float)(unsigned char)fgetc(file)*(1.0/255.0);
//			float r=(float)(unsigned char)fgetc(file)*(1.0/255.0);
			float b=gamma().b_U8_to_F32((unsigned char)fgetc(file));
			float g=gamma().g_U8_to_F32((unsigned char)fgetc(file));
			float r=gamma().r_U8_to_F32((unsigned char)fgetc(file));
			
			surface[h-y-1][x]=Color(
				r,
				g,
				b,
				1.0
			);
			if(bit_count==32)
				fgetc(file);
		}
	

	fclose(file);	
	return true;
}
