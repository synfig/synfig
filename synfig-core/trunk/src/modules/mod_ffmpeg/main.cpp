/*! ========================================================================
** Sinfg
** bleh
** $Id: main.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#define SINFG_MODULE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <sinfg/module.h>
#include "mptr_ffmpeg.h"
#include "trgt_ffmpeg.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_ffmpeg)
	MODULE_NAME("FFMPEG Module")
	MODULE_DESCRIPTION("Provides output targets for each format that FFMPEG supports")
	MODULE_AUTHOR("Robert B. Quattlebaum Jr.")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SINFG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_ffmpeg)
	BEGIN_TARGETS
		TARGET(ffmpeg_trgt)
		TARGET_EXT(ffmpeg_trgt,"avi")
		TARGET_EXT(ffmpeg_trgt,"mpg")
		TARGET_EXT(ffmpeg_trgt,"rm")
		TARGET_EXT(ffmpeg_trgt,"asf")
		TARGET_EXT(ffmpeg_trgt,"swf")
		TARGET_EXT(ffmpeg_trgt,"yuv")
		TARGET_EXT(ffmpeg_trgt,"rgb")
	END_TARGETS
//	BEGIN_IMPORTERS
//		IMPORTER_EXT(ffmpeg_mptr,"avi")
//		IMPORTER_EXT(ffmpeg_mptr,"mpg")
//		IMPORTER_EXT(ffmpeg_mptr,"mpeg")
//		IMPORTER_EXT(ffmpeg_mptr,"mov")
//		IMPORTER_EXT(ffmpeg_mptr,"rm")
//		IMPORTER_EXT(ffmpeg_mptr,"dv")
//	END_IMPORTERS
MODULE_INVENTORY_END
