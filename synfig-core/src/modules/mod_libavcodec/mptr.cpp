/* === S Y N F I G ========================================================= */
/*!	\file mptr.cpp
**	\brief writeme
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr.h"
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

SYNFIG_IMPORTER_INIT(Importer_LibAVCodec);
SYNFIG_IMPORTER_SET_NAME(Importer_LibAVCodec,"libav");
SYNFIG_IMPORTER_SET_EXT(Importer_LibAVCodec,"avi");
SYNFIG_IMPORTER_SET_VERSION(Importer_LibAVCodec,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(Importer_LibAVCodec,"$Id$");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(Importer_LibAVCodec, false);


/* === M E T H O D S ======================================================= */


Importer_LibAVCodec::Importer_LibAVCodec(const synfig::FileSystem::Identifier &identifier):
	Importer(identifier)
{
}

Importer_LibAVCodec::~Importer_LibAVCodec()
{
}

bool
Importer_LibAVCodec::get_frame(Surface &/*surface*/, const RendDesc &/*renddesc*/, Time /*time*/, ProgressCallback */*callback*/)
{
	return false;
}
