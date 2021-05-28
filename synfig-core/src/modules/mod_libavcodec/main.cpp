/* === S Y N F I G ========================================================= */
/*!	\file mod_libavcodec/main.cpp
**	\brief writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

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
