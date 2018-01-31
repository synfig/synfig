/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/surface.h
**	\brief Surface Header
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_SURFACE_H
#define __SYNFIG_RENDERING_SURFACE_H

/* === H E A D E R S ======================================================= */

#include <glibmm/threads.h>

#include <ETL/handle>

#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/token.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{


class Surface: public etl::shared_object
{
public:
	typedef etl::handle<Surface> Handle;
	typedef Surface* (*Fabric)();

	class Token: public synfig::Token {
	public:
		typedef ConstRef<Token> Handle;

		const Fabric fabric;
		const String name;

	private:
		template<typename T>
		T* fabric_template()
			{ return new T(); }

	public:
		template<typename TypeThis>
		Token(const String &name):
			synfig::Token(token),
			fabric(&fabric_template<TypeThis>),
			name(name)
		{ assert(fabric); }

		inline Handle handle() const
			{ return Handle(*this); }
	};

	static synfig::Token token;
	virtual Token::Handle get_token() const = 0;

private:
	bool blank;
	int width;
	int height;

protected:
	void set_desc(int width, int height, bool blank);

	virtual bool create_vfunc(int width, int height)
		{ return false; }
	virtual bool assign_vfunc(const Surface &other)
		{ return false; }
	virtual bool clear_vfunc()
		{ return false; }
	virtual bool reset_vfunc()
		{ return false; }
	//! Implementations of this function should to work quick
	virtual const Color* get_pixels_pointer_vfunc() const
		{ return NULL; }
	virtual bool get_pixels_vfunc(Color *dest) const;

public:
	Surface();
	virtual ~Surface();

	virtual bool is_read_only() const
		{ return false; }

	bool create(int width, int height);
	bool assign(const Surface &other);
	bool clear();
	bool reset();

	bool assign(const Color *pixels, int width, int height);
	bool assign(const Color *pixels)
		{ return assign(pixels, get_width(), get_height()); }

	bool touch();

	const Color* get_pixels_pointer() const;
	bool get_pixels(Color *dest) const;

	int get_width() const
		{ return width; }
	int get_height() const
		{ return height; }
	int get_pixels_count() const
		{ return get_width()*get_height(); }
	size_t get_buffer_size() const
		{ return get_pixels_count()*sizeof(Color); }
	bool is_exists() const
		{ return get_width() > 0 && get_height() > 0; }
	bool is_blank() const
		{ return blank || !is_exists(); }
};


class SurfaceResource: public etl::shared_object
{
public:
	typedef etl::handle<SurfaceResource> Handle;
	typedef std::map<Surface::Token::Handle, Surface::Handle> Map;

	class LockReadBase {
	private:
		Glib::Threads::RWLock::ReaderLock lock;
		Surface::Handle surface;
	public:
		LockReadBase(SurfaceResource &resource, Surface::Token::Handle token):
			lock(resource.rwlock), surface(resource.get_surface(token)) { }
		const Surface* get_surface() const
			{ return surface.get(); }
		operator bool () const
			{ return surface; }
	};

	class LockWriteBase {
	private:
		Glib::Threads::RWLock::WriterLock lock;
		Surface::Handle surface;
	public:
		LockWriteBase(SurfaceResource &resource, Surface::Token::Handle token):
			lock(resource.rwlock), surface(resource.get_surface(token))
		{
			if (surface) {
				resource.surfaces.clear();
				resource.surfaces[token] = surface;
				surface->touch();
			}
		}
		Surface* get_surface() const
			{ return surface.get(); }
		operator bool () const
			{ return surface; }
	};

	template<typename T>
	class LockRead: public LockReadBase {
	public:
		typedef T Type;
		LockRead(SurfaceResource &resource):
			LockReadBase(resource, Type::token) { }
		const Type* get() const
			{ return dynamic_cast<const Type*>(get_surface()); }
		const Type* operator->() const
			{ assert(get()); return get(); }
		const Type& operator*() const
			{ assert(get()); return *get(); }
	};

	template<typename T>
	class LockWrite: public LockWriteBase {
	public:
		typedef T Type;
		LockWrite(SurfaceResource &resource):
			LockWriteBase(resource, Type::token) { }
		Type* get() const
			{ return dynamic_cast<Type*>(get_surface()); }
		Type* operator->() const
			{ assert(get()); return get(); }
		Type& operator*() const
			{ assert(get()); return *get(); }
	};

private:
	int width;
	int height;
	Map surfaces;

	Glib::Threads::RWLock rwlock;

	Surface::Handle get_surface(Surface::Token::Handle token);

public:
	SurfaceResource();
	SurfaceResource(Surface::Handle surface);
	virtual ~SurfaceResource();

	void create(int width, int height);
	void assign(Surface::Handle surface);
	void clear();
	void reset();

	int get_width() const
		{ Glib::Threads::RWLock::ReaderLock lock(rwlock); return width; }
	int get_height() const
		{ Glib::Threads::RWLock::ReaderLock lock(rwlock); return height; }
	bool is_exists() const
		{ Glib::Threads::RWLock::ReaderLock lock(rwlock); return width > 0 && height > 0; }
	bool is_blank() const {
		Glib::Threads::RWLock::ReaderLock lock(rwlock);
		return surfaces.empty() || surfaces.begin()->second->is_blank();
	}

	bool has_surface(Surface::Token::Handle token) const {
		Glib::Threads::RWLock::ReaderLock lock(rwlock);
		return surfaces.count(token);
	}

	void set_size(const VectorInt &x)
		{ set_size(x[0], x[1]); }
	VectorInt get_size() const
		{ Glib::Threads::RWLock::ReaderLock lock(rwlock); return VectorInt(width, height); }
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
