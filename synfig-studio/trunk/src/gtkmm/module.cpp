/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: module.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "module.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
//using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Module::Module():status_(false)
{
}
	
Module::~Module()
{
	stop();
}

bool
Module::get_status()const
{
	return status_;
}

bool
Module::start()
{
	if(!get_status())
		status_=start_vfunc();
	return get_status();
}

bool
Module::stop()
{
	if(get_status() && count()<=1 && stop_vfunc())
	{
		status_=false;
		return true;
	}
	return false;
}
