/* === S Y N F I G ========================================================= */
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
