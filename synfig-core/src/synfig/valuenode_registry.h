/* === S Y N F I G ========================================================= */
/*!	\file valuenode_registry.h
**	\brief Valuenode automatic registry
**
**	\legal
**	Copyright (c) 2016 caryoscelus
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_REGISTRY_H
#define __SYNFIG_VALUENODE_REGISTRY_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/// This macro automatically registers valuenode and defines its
/// get_name & get_local_name methods.
#define REGISTER_VALUENODE(klass, _version, _name, _local_name) \
static bool result_##klass = synfig::ValueNodeRegistry::register_node_type(_name, \
	ValueNodeRegistry::localize_name(_local_name), _version, \
	reinterpret_cast<ValueNodeRegistry::Factory>(&klass::create), &klass::check_type); \
	\
	String klass::get_name() const { return _name; } \
	String klass::get_local_name() const { return _local_name; }

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNodeRegistry {
public:
	//! Type that represents a pointer to a ValueNode's constructor
	typedef LinkableValueNode* (*Factory)(const ValueBase&, etl::loose_handle<Canvas>);
	//! Pointer to check_type method
	typedef bool (*CheckType)(Type &type);

	struct BookEntry
	{
	private:
		String local_name;
	public:
		Factory factory;
		CheckType check_type;
		ReleaseVersion release_version; // which version of synfig introduced this valuenode type

		BookEntry(String local_name, Factory factory, CheckType check_type, ReleaseVersion release_version);
		String get_local_name() const { return localize_name(local_name); }
	};

	typedef std::map<String,BookEntry> Book;

public:
	static Book& book();
	static bool register_node_type(const String &name, const String &local_name, ReleaseVersion version, Factory factory, CheckType check_type);
	static bool cleanup();
	static String localize_name(const String &local_name);

public:
	//! Creates a Linkable Value Node based on the name and the returned
	//! value type. Returns a valid Handle if both (name and type) match
	static LinkableValueNode::Handle create(const String &name, const ValueBase& x);
	//! Each derived Linkable Value Node has to implement this function and
	//! should return true only if the type matches. \name is the name of
	//! the linked value node and \x is the returned value type
	static bool check_type(const String &name, Type &x);

private:
	static Book* book_;
}; // END of class ValueNodeRegistry

} // END of namespace synfig

/* === E N D =============================================================== */

#endif
