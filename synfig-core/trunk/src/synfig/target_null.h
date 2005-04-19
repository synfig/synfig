/* === S Y N F I G ========================================================= */
/*!	\file target_null.h
**	\brief Template Header
**
**	$Id: target_null.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_TARGET_NULL_H
#define __SYNFIG_TARGET_NULL_H

/* === H E A D E R S ======================================================= */

#include "target_scanline.h"
#include "general.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Target_Null
**	\brief A target which renders to nothing. Useful for benchmarks and other tests.
**	\todo writeme
*/
class Target_Null : public Target_Scanline
{
	Color *buffer;
	
	Target_Null():buffer(0) { }
	
public:

	~Target_Null() { delete buffer; } 

	virtual bool start_frame(ProgressCallback *cb=NULL)
		{ delete buffer; buffer=new Color[desc.get_w()*sizeof(Color)]; return true; }

	virtual void end_frame() { delete buffer; buffer=0; return; }

	virtual Color * start_scanline(int scanline) { return buffer; }

	virtual bool end_scanline() { return true; }
	
	static Target* create(const char *filename=0) { return new Target_Null(); }
}; // END of class Target_Null

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
