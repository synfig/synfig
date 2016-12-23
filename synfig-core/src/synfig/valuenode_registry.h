/* === S Y N F I G ========================================================= */
/*!	\file valuenode_registry.h
**	\brief Valuenode automatic registry
**
**	\legal
**	Copyright (c) 2016 caryoscelus
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_REGISTRY_H
#define __SYNFIG_VALUENODE_REGISTRY_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

#define VALUENODE_VERSION(klass, version) \
private: \
	friend class Registrable<klass>; \
	static const ReleaseVersion release_version = version; \
	klass() {}

#define REGISTRABLE_VALUENODE(klass) \
klass : public LinkableValueNode, public Registrable<klass>

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNodeRegistry {
public:
	//! Type that represents a pointer to a ValueNode's constructor
	using Factory = LinkableValueNode* (*)(const ValueBase&);
	//! Pointer to check_type method
	using CheckType = bool (*)(Type &type);

	struct BookEntry
	{
		String local_name;
		Factory factory;
		CheckType check_type;
		ReleaseVersion release_version; // which version of synfig introduced this valuenode type
	};

	using Book = std::map<String,BookEntry>;

public:
	static Book& book();
	static void register_node_type(String name, String local_name, ReleaseVersion version, Factory factory, CheckType check_type);
	static bool cleanup();

public:
	//! Creates a Linkable Value Node based on the name and the returned
	//! value type. Returns a valid Handle if both (name and type) match
	static LinkableValueNode::Handle create(const String &name, const ValueBase& x);
	//! Each derived Linkable Value Node has to implement this fucntion and
	//! should return true only if the type matches. \name is the name of
	//! the linked value node and \x is the returned value type
	static bool check_type(const String &name, Type &x);

private:
	static std::unique_ptr<Book> book_;
}; // END of class ValueNodeRegistry

// Automatically register class
// See http://stackoverflow.com/questions/401621/best-way-to-for-c-types-to-self-register-in-a-list
template<class T>
class Registrable {
private:
	struct do_register {
		do_register() {
			T t;
			ValueNodeRegistry::register_node_type(
				t.get_name(),
				t.get_local_name(),
				T::release_version,
				reinterpret_cast<ValueNodeRegistry::Factory>(&T::create),
				&T::check_type
			);
		}
	};
	// template magic to make sure do_register is instantiated
	template<do_register&> struct register_ref {};
	static do_register register_obj;
	static register_ref<register_obj> ref_obj;
}; // END of class Registrable

template<class T>
typename Registrable<T>::do_register Registrable<T>::register_obj;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
