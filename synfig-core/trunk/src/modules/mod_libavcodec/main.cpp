/* === S Y N F I G ========================================================= */
/*!	\file main.cpp
**	\brief writeme
**
**	$Id: main.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SYNFIG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/module.h>
#include <synfig/layer.h>

//#include "mptr.h"
#include "trgt_av.h" 

#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_libavcodec)
	MODULE_NAME("LibAVCodec Module (From FFMPEG)")
	MODULE_DESCRIPTION("Provides import/export ability for AVI, MPG, ASF, and a variety of other formats.")
	MODULE_AUTHOR("Adrian Bentley")
	MODULE_VERSION("0.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_libavcodec)
	BEGIN_TARGETS
		TARGET(Target_LibAVCodec)
		//TARGET_EXT(Target_LibAVCodec,"mpg")
		//TARGET_EXT(Target_LibAVCodec,"mpeg")
		//TARGET_EXT(Target_LibAVCodec,"mov")
		TARGET_EXT(Target_LibAVCodec,"asf")
		TARGET_EXT(Target_LibAVCodec,"rm")
		//TARGET_EXT(Target_LibAVCodec,"mpg")
		TARGET_EXT(Target_LibAVCodec,"wmv")
		TARGET_EXT(Target_LibAVCodec,"yuv")
		//TARGET_EXT(Target_LibAVCodec,"dv")
	END_TARGETS
	BEGIN_IMPORTERS
//		IMPORTER(bmp_mptr)
	END_IMPORTERS
MODULE_INVENTORY_END
