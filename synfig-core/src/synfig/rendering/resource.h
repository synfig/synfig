/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/resource.h
**	\brief Resource Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_RESOURCE_H
#define __SYNFIG_RENDERING_RESOURCE_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <ETL/handle>
#include <mutex>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

// TODO: move to ::synfig or to ::etl
class Resource: public etl::shared_object
{
public:
	typedef unsigned long long Id;

private:
	static Id last_id;

public:
	typedef etl::handle<Resource> Handle;
	typedef std::vector<Handle> List;

	class Storage: public etl::virtual_shared_object
	{
	public:
		typedef etl::handle<Storage> Handle;

	private:
		mutable int refcount;
		#ifdef ETL_LOCK_REFCOUNTS
		mutable std::mutex mtx;
		#endif

		friend class Resource;

		List resources;

		static const List blank;

	public:
		Storage();
		~Storage();

		virtual void ref() const;
		virtual bool unref_inactive() const;
		virtual bool unref() const;
		virtual int count() const;
	};

private:
	const Id id;
	mutable Storage::Handle alternatives;
	mutable std::mutex get_alternative_mtx;

public:
	Resource(): id(++last_id) { }

	Id get_id() const { return id; }

	const List& get_alternatives() const
		{ return alternatives ? alternatives->resources : Storage::blank; }

	void set_alternative(Handle other) const;
	void unset_alternative() const;

	template<typename T>
	etl::handle<T> find_alternative() const
	{
		const List &resources = get_alternatives();
		for(List::const_iterator i = resources.begin(); i != resources.end(); ++i)
			if (etl::handle<T> alternative = etl::handle<T>::cast_dynamic(*i))
				return alternative;
		return etl::handle<T>();
	}

	template<typename T>
	etl::handle<T> get_alternative() const
	{
		std::lock_guard<std::mutex> lock(get_alternative_mtx);
		etl::handle<T> alternative = find_alternative<T>();
		if (!alternative)
		{
			alternative = new T(*this);
			set_alterntive(alternative);
		}
		return alternative;
	}
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
