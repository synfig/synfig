/* === S I N F G =========================================================== */
/*!	\file mptr.cpp
**	\brief writeme
**
**	$Id: mptr.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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

#define SINFG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr.h"
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

SINFG_IMPORTER_INIT(Importer_LibAVCodec);
SINFG_IMPORTER_SET_NAME(Importer_LibAVCodec,"Importer_LibAVCodec");
SINFG_IMPORTER_SET_EXT(Importer_LibAVCodec,"avi");
SINFG_IMPORTER_SET_VERSION(Importer_LibAVCodec,"0.1");
SINFG_IMPORTER_SET_CVS_ID(Importer_LibAVCodec,"$Id: mptr.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $");

/* === M E T H O D S ======================================================= */


Importer_LibAVCodec::Importer_LibAVCodec(const char *file):
	filename(file)
{
}

Importer_LibAVCodec::~Importer_LibAVCodec()
{
}

bool
Importer_LibAVCodec::get_frame(sinfg::Surface &surface,Time, sinfg::ProgressCallback *cb)
{
	return false;
}
