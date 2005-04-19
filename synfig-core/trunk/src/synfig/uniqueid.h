/* === S Y N F I G ========================================================= */
/*!	\file uniqueid.h
**	\brief Template Header
**
**	$Id: uniqueid.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_UNIQUEID_H
#define __SYNFIG_UNIQUEID_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
	
class UniqueIDLessThan;
	
/*! \class UniqueID
**	\brief \todo
*/
class UniqueID
{
	friend class UniqueIDLessThan;
	
	int id_;
	
	explicit UniqueID(int id_):id_(id_) { }
	
	static int next_id();
	
public:
	
	//! Returns the internal unique identifier for this object.
	/*! The return value from this isn't really useful for
	**	much other than debug output. Nonetheless, that is
	**	one step above useless, so here it is. */
	const int &get_uid()const { return id_; }
	
	UniqueID():id_(next_id()) { }
	
	void make_unique() { id_=next_id(); }
	
	static const UniqueID nil() { return UniqueID(0); }
	
	operator bool()const { return static_cast<bool>(id_); }

	void mimic(const UniqueID& x) { id_=x.id_; }
	
	bool operator==(const UniqueID &rhs)const { return id_==rhs.id_; }
	bool operator!=(const UniqueID &rhs)const { return id_!=rhs.id_; }
	bool operator<(const UniqueID &rhs)const { return id_<rhs.id_; }
}; // END of class UniqueID

/*! \class UniqueIDLessThan
**	\brief A function class used for sorting based on UniqueIDs
*/
class UniqueIDLessThan
{
public:
	bool operator()(const UniqueID &lhs, const UniqueID &rhs)const
	{ return lhs.id_<rhs.id_; }
};


}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
