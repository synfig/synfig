/* === S I N F G =========================================================== */
/*!	\file transform.h
**	\brief Template Header
**
**	$Id: transform.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SINFG_TRANSFORM_H
#define __SINFG_TRANSFORM_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include "vector.h"
#include <list>
#include "rect.h"
#include "guid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class Transform : public etl::shared_object
{
	GUID guid_;
	
public:
	typedef etl::handle<Transform> Handle;

protected:
	Transform(const GUID& guid=GUID(0)):guid_(guid) { }

public:

	const GUID& get_guid()const { return guid_; }

	virtual ~Transform() { }
	virtual sinfg::Vector perform(const sinfg::Vector& x)const=0;
	virtual sinfg::Vector unperform(const sinfg::Vector& x)const=0;

	virtual sinfg::Rect perform(const sinfg::Rect& x)const;
	virtual sinfg::Rect unperform(const sinfg::Rect& x)const;

}; // END of class Transform

class TransformStack : public std::list<Transform::Handle>
{
public:
	GUID get_guid()const;

	sinfg::Vector perform(const sinfg::Vector& x)const;
	sinfg::Vector unperform(const sinfg::Vector& x)const;
	
	sinfg::Rect perform(const sinfg::Rect& x)const;
	sinfg::Rect unperform(const sinfg::Rect& x)const;
	
	void push(const Transform::Handle& x) { if(x)push_back(x); }
	void pop() { pop_back(); }
}; // END of class TransformStack

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
