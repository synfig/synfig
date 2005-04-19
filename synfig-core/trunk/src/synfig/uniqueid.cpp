/* === S Y N F I G ========================================================= */
/*!	\file uniqueid.cpp
**	\brief Template File
**
**	$Id: uniqueid.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#include "uniqueid.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

static int uniqueid_pool_(0);

/* === M E T H O D S ======================================================= */

int
synfig::UniqueID::next_id()
{
	return ++uniqueid_pool_;
}
