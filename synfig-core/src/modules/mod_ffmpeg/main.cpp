/* === S Y N F I G ========================================================= */
/*!	\file mod_ffmpeg/main.cpp
**	\brief Template Header
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

#define SYNFIG_MODULE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/module.h>
#include "mptr_ffmpeg.h"
#include "trgt_ffmpeg.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_ffmpeg)
	MODULE_NAME("FFMPEG Module")
	MODULE_DESCRIPTION("Provides output targets for each format that FFMPEG supports")
	MODULE_AUTHOR("Robert B. Quattlebaum Jr.")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_ffmpeg)
	BEGIN_TARGETS
		TARGET(ffmpeg_trgt)
		TARGET_EXT(ffmpeg_trgt,"avi")
		TARGET_EXT(ffmpeg_trgt,"flv")
		TARGET_EXT(ffmpeg_trgt,"mkv")
		TARGET_EXT(ffmpeg_trgt,"mpg")
		TARGET_EXT(ffmpeg_trgt,"mpeg")
		TARGET_EXT(ffmpeg_trgt,"mp4")
		TARGET_EXT(ffmpeg_trgt,"ogv")
		TARGET_EXT(ffmpeg_trgt,"rgb")
		TARGET_EXT(ffmpeg_trgt,"wmv")
		TARGET_EXT(ffmpeg_trgt,"yuv")
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER_EXT(ffmpeg_mptr,"avi")
		IMPORTER_EXT(ffmpeg_mptr,"mp4")
		IMPORTER_EXT(ffmpeg_mptr,"gif")
		IMPORTER_EXT(ffmpeg_mptr,"mpg")
		IMPORTER_EXT(ffmpeg_mptr,"mpeg")
		IMPORTER_EXT(ffmpeg_mptr,"mov")
		IMPORTER_EXT(ffmpeg_mptr,"rm")
		IMPORTER_EXT(ffmpeg_mptr,"dv")
	END_IMPORTERS
MODULE_INVENTORY_END
