/* === S I N F G =========================================================== */
/*!	\file protocol.h
**	\brief Template Header
**
**	$Id: template.h,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $
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

#ifndef __SINFG_PROTOCOL_H
#define __SINFG_PROTOCOL_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class Object
{
public:
	
	sigc::signal_
	bool find_protocol(Protocol& proto)
	{
		
	}
};

class Protocol
{
public:
	class Type;
	
}; // END of class Protocol

class Protocol::Type
{
}; // END of class Protocol::Type

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
