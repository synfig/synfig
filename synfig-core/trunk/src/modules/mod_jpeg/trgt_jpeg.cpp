/*! ========================================================================
** Sinfg
** jpeg_trgt Target Module
** $Id: trgt_jpeg.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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

#define SINFG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "trgt_jpeg.h"
#include <jpeglib.h>
#include <ETL/stringf>
#include <cstdio>
#include <algorithm>
#include <functional>
#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SINFG_TARGET_INIT(jpeg_trgt);
SINFG_TARGET_SET_NAME(jpeg_trgt,"jpeg");
SINFG_TARGET_SET_EXT(jpeg_trgt,"jpg");
SINFG_TARGET_SET_VERSION(jpeg_trgt,"0.1");
SINFG_TARGET_SET_CVS_ID(jpeg_trgt,"$Id: trgt_jpeg.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $");

/* === M E T H O D S ======================================================= */

jpeg_trgt::jpeg_trgt(const char *Filename)
{
	file=NULL;
	filename=Filename;
	buffer=NULL;
	ready=false;
	quality=95;
	color_buffer=0;	
	set_remove_alpha();
}

jpeg_trgt::~jpeg_trgt()
{
	if(ready)
	{
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		ready=false;
	}
	if(file)
		fclose(file);
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
jpeg_trgt::set_rend_desc(RendDesc *given_desc)
{
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}

bool
jpeg_trgt::start_frame(sinfg::ProgressCallback *callback)
{
	int w=desc.get_w(),h=desc.get_h();
	
	if(file && file!=stdout)
		fclose(file);
	if(filename=="-")
	{
		if(callback)callback->task(strprintf("(stdout) %d",imagecount).c_str());
		file=stdout;
	}
	else if(multi_image)
	{
		String
			newfilename(filename),
			ext(find(filename.begin(),filename.end(),'.'),filename.end());
		newfilename.erase(find(newfilename.begin(),newfilename.end(),'.'),newfilename.end());
		
		newfilename+=etl::strprintf("%04d",imagecount)+ext;
		file=fopen(newfilename.c_str(),"wb");
		if(callback)callback->task(newfilename);
	}
	else
	{
		file=fopen(filename.c_str(),"wb");
		if(callback)callback->task(filename);
	}
	
	if(!file)
		return false;
		
	delete [] buffer;
	buffer=new unsigned char[3*w];
	
	delete [] color_buffer;
	color_buffer=new Color[w];


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file);

	cinfo.image_width = w; 	/* image width and height, in pixels */
	cinfo.image_height = h;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	
	/* Step 4: Start compressor */
	
	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);
		
	ready=true;
	return true;
}

void
jpeg_trgt::end_frame()
{
	if(ready)
	{
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		ready=false;
	}

	if(file && file!=stdout)
		fclose(file);
	file=NULL;
	imagecount++;
}

Color *
jpeg_trgt::start_scanline(int scanline)
{
	return color_buffer;
}

bool
jpeg_trgt::end_scanline()
{
	if(!file || !ready)
		return false;

	convert_color_format(buffer, color_buffer, desc.get_w(), PF_RGB,gamma());
	JSAMPROW *row_pointer(&buffer);
	jpeg_write_scanlines(&cinfo, row_pointer, 1);

	return true;
}
