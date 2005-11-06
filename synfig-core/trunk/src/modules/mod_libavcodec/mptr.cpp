/* === S Y N F I G ========================================================= */
/*!	\file mptr.cpp
**	\brief writeme
**
**	$Id: mptr.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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

#define SYNFIG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr.h"
#include <synfig/general.h>
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
SYNFIG_IMPORTER_SET_NAME(Importer_LibAVCodec,"Importer_LibAVCodec");
SYNFIG_IMPORTER_SET_EXT(Importer_LibAVCodec,"avi");
SYNFIG_IMPORTER_SET_VERSION(Importer_LibAVCodec,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(Importer_LibAVCodec,"$Id: mptr.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $");

/* === M E T H O D S ======================================================= */


Importer_LibAVCodec::Importer_LibAVCodec(const char *file):
	filename(file)
{
}

Importer_LibAVCodec::~Importer_LibAVCodec()
{
}

bool
Importer_LibAVCodec::get_frame(synfig::Surface &surface,Time, synfig::ProgressCallback *cb)
{
	return false;
}
