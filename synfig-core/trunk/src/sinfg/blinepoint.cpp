/* === S I N F G =========================================================== */
/*!	\file blinepoint.cpp
**	\brief Template File
**
**	$Id: blinepoint.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "blinepoint.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
sinfg::BLinePoint::reverse()
{
	if(split_tangent_)
	{
		std::swap(tangent_[0],tangent_[1]);
		tangent_[0]=-tangent_[0];
		tangent_[1]=-tangent_[1];
	}
	else
	{
		tangent_[0]=-tangent_[0];
		tangent_[1]=-tangent_[1];
	}
}
