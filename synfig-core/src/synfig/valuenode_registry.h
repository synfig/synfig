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

/// This macro automatically registers valuenode and defines its
/// get_name & get_local_name functions.
/// NOTE: this relies on valuenode being either in synfig namespace
/// or in 'used' namespace.
/// TODO: move all nodes into synfig::valuenodes namespace
#define REGISTER_VALUENODE(klass, _version, _name, _local_name)				\
	namespace synfig 														\
	{																		\
		class Register_##klass:												\
			public RegisterValueNode<klass, Register_##klass>				\
		{																	\
			public:															\
				static const ReleaseVersion release_version;				\
				static const char* name;									\
				static const char* local_name;								\
		};																	\
																			\
		String																\
		klass::get_name() const												\
		{																	\
			return _name;													\
		}																	\
																			\
		String																\
		klass::get_local_name()const										\
		{																	\
			return _local_name;												\
		}																	\
																			\
		const ReleaseVersion Register_##klass::release_version = _version;	\
		const char* Register_##klass::name = _name;							\
		const char* Register_##klass::local_name = _local_name;				\
	}

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
		String local_name;
		Factory factory;
		CheckType check_type;
		ReleaseVersion release_version; // which version of synfig introduced this valuenode type
	};

	typedef std::map<String,BookEntry> Book;

public:
	static Book& book();
	static void register_node_type(const String &name, const String &local_name, ReleaseVersion version, Factory factory, CheckType check_type);
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

// Automatically register class
// See http://stackoverflow.com/questions/401621/best-way-to-for-c-types-to-self-register-in-a-list
template<class NodeT, class SelfT>
class RegisterValueNode {
private:
	struct do_register {
		do_register()
		{
			ValueNodeRegistry::register_node_type(
				SelfT::name,
				ValueNodeRegistry::localize_name(SelfT::local_name),
				SelfT::release_version,
				reinterpret_cast<ValueNodeRegistry::Factory>(&NodeT::create),
				&NodeT::check_type
			);
		}
	};
	// template magic to make sure do_register is instantiated
	template<do_register&> struct register_ref {};
	static do_register register_obj;
	static register_ref<register_obj> ref_obj;
}; // END of class RegisterValueNode

template<class NodeT, class SelfT>
typename RegisterValueNode<NodeT, SelfT>::do_register RegisterValueNode<NodeT, SelfT>::register_obj;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
