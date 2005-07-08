/* === S Y N F I G ========================================================= */
/*!	\file module.h
**	\brief Template Header
**
**	$Id: module.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MODULE_H
#define __SYNFIG_MODULE_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Module : public etl::shared_object
{
	bool status_;
	
protected:
	Module();

public:
	virtual ~Module();

	bool start();
	
	bool stop();
	
	bool get_status()const;

protected:
	
	virtual bool start_vfunc()=0;
	virtual bool stop_vfunc()=0;
};

	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
