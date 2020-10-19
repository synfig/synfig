/* === S Y N F I G ========================================================= */
/*!	\file token.h
**	\brief Token Header
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahonin
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

#ifndef __SYNFIG_TOKEN_H
#define __SYNFIG_TOKEN_H

/* === H E A D E R S ======================================================= */

#include <set>
#include <map>
#include <cassert>

#include "string.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

template<typename T>
class ConstRef {
public:
	typedef T Type;

private:
	friend class ConstRefHelper;
	const Type *pointer;

public:
	inline ConstRef():
		pointer() { }
	inline explicit ConstRef(const Type &x):
		pointer(&x) { }

	template<typename TT>
	inline ConstRef(const ConstRef<TT> &x):
		pointer() { *this = x.template base<Type>(); }

	template<typename TT>
	inline ConstRef<TT> base() const
		{ return pointer ? ConstRef<TT>(*pointer) : ConstRef<TT>(); }

	template<typename TT>
	inline ConstRef<TT> as() const
	{
		TT *p = dynamic_cast<TT*>(pointer);
		return p ? ConstRef<TT>(*p) : ConstRef<TT>();
	}

	inline void reset()
		{ *pointer = NULL; }
	inline bool is_valid() const
		{ return pointer; }

	inline const Type* operator-> () const
		{ assert(pointer); return pointer; }
	inline operator bool() const
		{ return is_valid(); }

	inline bool operator== (const ConstRef& other) const
		{ return pointer == other.pointer; }
	inline bool operator!= (const ConstRef& other) const
		{ return pointer != other.pointer; }
	inline bool operator< (const ConstRef& other) const
		{ return pointer < other.pointer; }
};


class Token {
public:
	typedef ConstRef<Token> Handle;
	typedef std::set<Handle> Set;
	typedef std::map<String, String> MetaData;

	class SetBuilder {
	public:
		Set set;
		inline SetBuilder() { }
		inline explicit SetBuilder(Handle x)
			{ add(x); }
		inline SetBuilder& add(Handle x)
			{ set.insert(x); return *this; }
	};


private:
	static bool ready_;
	static bool root_exists_;
	static Token *first_;
	static Token *last_;
	Token *previous_;
	Token *next_;

	bool in_process_;
	bool prepared_;

	Set parents_;
	Set children_;
	Set all_parents_;
	Set all_children_;

	Token(const Token&);
	Token& operator= (const Token&) { return *this; }

	inline Token& cast_const() const
		{ return *const_cast<Token*>(this); }

	void init();
	void fill_all_parents();

protected:
	virtual void prepare_vfunc() { }
	virtual void unprepare_vfunc() { }

public:
	static Token token;

	Token();
	explicit Token(const Handle &parent);
	explicit Token(const Set &parents);
	virtual ~Token();

	inline Handle handle() const
		{ return Handle(*this); }

	inline const Set& parents() const
		{ assert(ready_); return parents_; }
	inline const Set& children() const
		{ assert(ready_); return children_; }
	inline const Set& all_parents() const
		{ assert(ready_); return all_parents_; }
	inline const Set& all_children() const
		{ assert(ready_); return all_children_; }

	inline bool is_parent_of(Handle token) const
		{ return all_children().count(token); }
	inline bool is_child_of(Handle token) const
		{ return all_parents().count(token); }
	inline bool is_equal(Handle token) const
		{ return handle() == token; }
	inline bool is(Handle token) const
		{ return is_equal(token) || is_child_of(token); }

	void prepare();
	static void rebuild();
};

}; // END of namespace synfig

/*
===============================================================================

Sample implementation for Layers

===============================================================================

class MyLayer {
public:
	typedef MyLayer* (*Fabric)();

	class Token: public synfig::Token {
	public:
		typedef ConstRef<Token> Handle;

		const Fabric fabric;
		const String name;
		const String category;

	private:
		template<typename T>
		T* fabric_template()
			{ return new T(); }

	public:
		template<typename TypeThis, typename TypeParent>
		Token(
			const String &name,
			const String &category = String()
		):
			synfig::Token(TypeParent::token.handle()),
			fabric(&fabric_template<TypeThis>),
			name(name),
			category(category)
		{ }

		inline Handle handle() const
			{ return Handle(*this); }
	};

	static synfig::Token token;

	virtual ~MyLayer() { }
	virtual Token::Handle get_token() = 0;
};

synfig::Token MyLayer::token;

class MyRegion: public MyLayer {
public:
	static Token token;
	virtual Token::Handle get_token()
		{ return token.handle(); }
};

MyLayer::Token MyRegion::token<MyRegion, MyLayer>(
	"region",
	"common" );

===============================================================================
*/

/* === E N D =============================================================== */

#endif
