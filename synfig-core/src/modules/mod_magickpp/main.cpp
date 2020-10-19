/* === S Y N F I G ========================================================= */
/*!	\file mod_magickpp/main.cpp
**	\brief Magick++ plugin
**
**	$Id$
**
**	\legal
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <synfig/general.h>

#include <synfig/module.h>
#include "trgt_magickpp.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_magickpp)
	MODULE_NAME("Magick++ Module (libMagick++)")
	MODULE_DESCRIPTION("Provides an animated GIF target")
	MODULE_AUTHOR("Chris Moore")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_magickpp)
	BEGIN_TARGETS
		TARGET(magickpp_trgt)
		TARGET_EXT(magickpp_trgt, "8bim")
		TARGET_EXT(magickpp_trgt, "8bimtext")
		TARGET_EXT(magickpp_trgt, "8bimwtext")
		TARGET_EXT(magickpp_trgt, "a")
		TARGET_EXT(magickpp_trgt, "app1")
		TARGET_EXT(magickpp_trgt, "app1jpeg")
		TARGET_EXT(magickpp_trgt, "art")
		TARGET_EXT(magickpp_trgt, "avs")
		TARGET_EXT(magickpp_trgt, "b")
		TARGET_EXT(magickpp_trgt, "bie")
		TARGET_EXT(magickpp_trgt, "bmp")
		TARGET_EXT(magickpp_trgt, "bmp2")
		TARGET_EXT(magickpp_trgt, "bmp3")
		TARGET_EXT(magickpp_trgt, "c")
		TARGET_EXT(magickpp_trgt, "cache")
		TARGET_EXT(magickpp_trgt, "cin")
		TARGET_EXT(magickpp_trgt, "cip")
		TARGET_EXT(magickpp_trgt, "clip")
		TARGET_EXT(magickpp_trgt, "clipboard")
		TARGET_EXT(magickpp_trgt, "cmyk")
		TARGET_EXT(magickpp_trgt, "cmyka")
		TARGET_EXT(magickpp_trgt, "cur")
		TARGET_EXT(magickpp_trgt, "dcx")
		TARGET_EXT(magickpp_trgt, "dib")
		TARGET_EXT(magickpp_trgt, "dpx")
		TARGET_EXT(magickpp_trgt, "emf")
		TARGET_EXT(magickpp_trgt, "epdf")
		TARGET_EXT(magickpp_trgt, "epi")
		TARGET_EXT(magickpp_trgt, "eps")
		TARGET_EXT(magickpp_trgt, "eps2")
		TARGET_EXT(magickpp_trgt, "eps3")
		TARGET_EXT(magickpp_trgt, "epsf")
		TARGET_EXT(magickpp_trgt, "epsi")
		TARGET_EXT(magickpp_trgt, "ept")
		TARGET_EXT(magickpp_trgt, "ept2")
		TARGET_EXT(magickpp_trgt, "ept3")
		TARGET_EXT(magickpp_trgt, "exif")
		TARGET_EXT(magickpp_trgt, "exr")
		TARGET_EXT(magickpp_trgt, "fax")
		TARGET_EXT(magickpp_trgt, "file")
		TARGET_EXT(magickpp_trgt, "fits")
		TARGET_EXT(magickpp_trgt, "fpx")
		TARGET_EXT(magickpp_trgt, "ftp")
		TARGET_EXT(magickpp_trgt, "fts")
		TARGET_EXT(magickpp_trgt, "g")
		TARGET_EXT(magickpp_trgt, "g3")
		TARGET_EXT(magickpp_trgt, "gif")
		TARGET_EXT(magickpp_trgt, "gif87")
		TARGET_EXT(magickpp_trgt, "granite")
		TARGET_EXT(magickpp_trgt, "gray")
		TARGET_EXT(magickpp_trgt, "h")
		TARGET_EXT(magickpp_trgt, "histogram")
		TARGET_EXT(magickpp_trgt, "htm")
		TARGET_EXT(magickpp_trgt, "html")
		TARGET_EXT(magickpp_trgt, "http")
		TARGET_EXT(magickpp_trgt, "icb")
		TARGET_EXT(magickpp_trgt, "icc")
		TARGET_EXT(magickpp_trgt, "icm")
		TARGET_EXT(magickpp_trgt, "ico")
		TARGET_EXT(magickpp_trgt, "icon")
		TARGET_EXT(magickpp_trgt, "info")
		TARGET_EXT(magickpp_trgt, "ipl")
		TARGET_EXT(magickpp_trgt, "iptc")
		TARGET_EXT(magickpp_trgt, "iptctext")
		TARGET_EXT(magickpp_trgt, "iptcwtext")
		TARGET_EXT(magickpp_trgt, "jbg")
		TARGET_EXT(magickpp_trgt, "jbig")
		TARGET_EXT(magickpp_trgt, "jng")
		TARGET_EXT(magickpp_trgt, "jp2")
		TARGET_EXT(magickpp_trgt, "jpc")
		TARGET_EXT(magickpp_trgt, "jpeg")
		TARGET_EXT(magickpp_trgt, "jpg")
		TARGET_EXT(magickpp_trgt, "jpx")
		TARGET_EXT(magickpp_trgt, "k")
		TARGET_EXT(magickpp_trgt, "logo")
		TARGET_EXT(magickpp_trgt, "m")
		TARGET_EXT(magickpp_trgt, "m2v")
		TARGET_EXT(magickpp_trgt, "magick")
		TARGET_EXT(magickpp_trgt, "map")
		TARGET_EXT(magickpp_trgt, "mat")
		TARGET_EXT(magickpp_trgt, "matte")
		TARGET_EXT(magickpp_trgt, "miff")
		TARGET_EXT(magickpp_trgt, "mng")
		TARGET_EXT(magickpp_trgt, "mono")
		TARGET_EXT(magickpp_trgt, "mpc")
		TARGET_EXT(magickpp_trgt, "mpeg")
		TARGET_EXT(magickpp_trgt, "mpg")
		TARGET_EXT(magickpp_trgt, "mpr")
		TARGET_EXT(magickpp_trgt, "mpri")
		TARGET_EXT(magickpp_trgt, "msl")
		TARGET_EXT(magickpp_trgt, "msvg")
		TARGET_EXT(magickpp_trgt, "mtv")
		TARGET_EXT(magickpp_trgt, "mvg")
		TARGET_EXT(magickpp_trgt, "netscape")
		TARGET_EXT(magickpp_trgt, "null")
		TARGET_EXT(magickpp_trgt, "o")
		TARGET_EXT(magickpp_trgt, "otb")
		TARGET_EXT(magickpp_trgt, "pal")
		TARGET_EXT(magickpp_trgt, "palm")
		TARGET_EXT(magickpp_trgt, "pam")
		TARGET_EXT(magickpp_trgt, "pbm")
		TARGET_EXT(magickpp_trgt, "pcd")
		TARGET_EXT(magickpp_trgt, "pcds")
		TARGET_EXT(magickpp_trgt, "pcl")
		TARGET_EXT(magickpp_trgt, "pct")
		TARGET_EXT(magickpp_trgt, "pcx")
		TARGET_EXT(magickpp_trgt, "pdb")
		TARGET_EXT(magickpp_trgt, "pdf")
		TARGET_EXT(magickpp_trgt, "pfm")
		TARGET_EXT(magickpp_trgt, "pgm")
		TARGET_EXT(magickpp_trgt, "picon")
		TARGET_EXT(magickpp_trgt, "pict")
		TARGET_EXT(magickpp_trgt, "pjpeg")
		TARGET_EXT(magickpp_trgt, "pm")
		TARGET_EXT(magickpp_trgt, "png")
		TARGET_EXT(magickpp_trgt, "png24")
		TARGET_EXT(magickpp_trgt, "png32")
		TARGET_EXT(magickpp_trgt, "png8")
		TARGET_EXT(magickpp_trgt, "pnm")
		TARGET_EXT(magickpp_trgt, "ppm")
		TARGET_EXT(magickpp_trgt, "preview")
		TARGET_EXT(magickpp_trgt, "ps")
		TARGET_EXT(magickpp_trgt, "ps2")
		TARGET_EXT(magickpp_trgt, "ps3")
		TARGET_EXT(magickpp_trgt, "psd")
		TARGET_EXT(magickpp_trgt, "ptif")
		TARGET_EXT(magickpp_trgt, "r")
		TARGET_EXT(magickpp_trgt, "ras")
		TARGET_EXT(magickpp_trgt, "rgb")
		TARGET_EXT(magickpp_trgt, "rgba")
		TARGET_EXT(magickpp_trgt, "rgbo")
		TARGET_EXT(magickpp_trgt, "rose")
		TARGET_EXT(magickpp_trgt, "sgi")
		TARGET_EXT(magickpp_trgt, "shtml")
		TARGET_EXT(magickpp_trgt, "sun")
		TARGET_EXT(magickpp_trgt, "svg")
		TARGET_EXT(magickpp_trgt, "svgz")
		TARGET_EXT(magickpp_trgt, "text")
		TARGET_EXT(magickpp_trgt, "tga")
		TARGET_EXT(magickpp_trgt, "thumbnail")
		TARGET_EXT(magickpp_trgt, "tif")
		TARGET_EXT(magickpp_trgt, "tiff")
		TARGET_EXT(magickpp_trgt, "txt")
		TARGET_EXT(magickpp_trgt, "uil")
		TARGET_EXT(magickpp_trgt, "uyvy")
		TARGET_EXT(magickpp_trgt, "vda")
		TARGET_EXT(magickpp_trgt, "vicar")
		TARGET_EXT(magickpp_trgt, "vid")
		TARGET_EXT(magickpp_trgt, "viff")
		TARGET_EXT(magickpp_trgt, "vst")
		TARGET_EXT(magickpp_trgt, "wbmp")
		TARGET_EXT(magickpp_trgt, "wmfwin32")
		TARGET_EXT(magickpp_trgt, "x")
		TARGET_EXT(magickpp_trgt, "xbm")
		TARGET_EXT(magickpp_trgt, "xmp")
		TARGET_EXT(magickpp_trgt, "xpm")
		TARGET_EXT(magickpp_trgt, "xv")
		TARGET_EXT(magickpp_trgt, "xwd")
		TARGET_EXT(magickpp_trgt, "y")
		TARGET_EXT(magickpp_trgt, "ycbcr")
		TARGET_EXT(magickpp_trgt, "ycbcra")
		TARGET_EXT(magickpp_trgt, "yuv")
	END_TARGETS
MODULE_INVENTORY_END
