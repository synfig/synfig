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
#include <synfig/rect.h>

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

	template<typename TypeLock, typename TypeSurface, bool for_write>
	class LockBase {
	public: // keep this order of fields (it's a construction order)
		const Handle resource;
	private:
		const TypeLock lock;
	public:
		const Surface::Handle surface;

		explicit LockBase(
			const Handle &resource,
			const Surface::Token::Handle &token = Surface::Token::Handle()
		):
			resource(resource),
			lock(resource->rwlock),
			surface(resource->get_surface(token, for_write, true, RectInt())) { }
		LockBase(
			const Handle &resource,
			const Surface::Token::Handle &token,
			const RectInt &rect
		):
			lock(resource->rwlock),
			surface(resource->get_surface(token, for_write, false, rect)) { }
		const Handle& get_resource() const
			{ return resource; }
		TypeSurface* get_surface() const
			{ return surface.get(); }
		operator bool () const
			{ return surface; }
	};

	typedef LockBase<Glib::Threads::RWLock::ReaderLock, const Surface, false> LockReadBase;
	typedef LockBase<Glib::Threads::RWLock::WriterLock, Surface, true> LockWriteBase;
	typedef LockBase<Glib::Threads::RWLock::ReaderLock, Surface, true> SemiLockWriteBase;

	template<typename T>
	class LockRead: public LockReadBase {
	public:
		typedef T Type;
		explicit LockRead(const Handle &resource):
			LockReadBase(resource, Type::token.handle()) { }
		LockRead(const Handle &resource, const RectInt &rect):
			LockReadBase(resource, Type::token.handle(), rect) { }
		LockRead(const Handle &resource, const Surface::Token::Handle &token):
			LockReadBase(resource, token) { }
		LockRead(const Handle &resource, const Surface::Token::Handle &token, const RectInt &rect):
			LockReadBase(resource, token, rect) { }
		etl::handle<Type> get() const
			{ return etl::handle<Type>::cast_dynamic(surface); }
		const Type* operator->() const
			{ assert(get()); return surface.get(); }
		const Type& operator*() const
			{ assert(get()); return *surface; }
	};

	template<typename T>
	class LockWrite: public LockWriteBase {
	public:
		typedef T Type;
		explicit LockWrite(const Handle &resource):
			LockWriteBase(resource, Type::token) { }
		LockWrite(const Handle &resource, const RectInt &rect):
			LockWriteBase(resource, Type::token, rect) { }
		LockWrite(const Handle &resource, const Surface::Token::Handle &token):
			LockWriteBase(resource, token) { }
		LockWrite(const Handle &resource, const Surface::Token::Handle &token, const RectInt &rect):
			LockWriteBase(resource, token, rect) { }
		etl::handle<Type> get() const
			{ return etl::handle<Type>::cast_dynamic(surface); }
		Type* operator->() const
			{ assert(get()); return surface.get(); }
		Type& operator*() const
			{ assert(get()); return *surface; }
	};

	template<typename T>
	class SemiLockWrite: public SemiLockWriteBase {
	public:
		typedef T Type;
		explicit SemiLockWrite(const Handle &resource):
			SemiLockWriteBase(resource, Type::token) { }
		SemiLockWrite(const Handle &resource, const RectInt &rect):
			SemiLockWriteBase(resource, Type::token, rect) { }
		SemiLockWrite(const Handle &resource, const Surface::Token::Handle &token):
			SemiLockWriteBase(resource, token) { }
		SemiLockWrite(const Handle &resource, const Surface::Token::Handle &token, const RectInt &rect):
			SemiLockWriteBase(resource, token, rect) { }
		etl::handle<Type> get() const
			{ return etl::handle<Type>::cast_dynamic(surface); }
		Type* operator->() const
			{ assert(get()); return surface.get(); }
		Type& operator*() const
			{ assert(get()); return *surface; }
	};

private:
	int width;
	int height;
	bool blank;
	Map surfaces;

	Glib::Threads::Mutex mutex;
	Glib::Threads::RWLock rwlock;

	Surface::Handle get_surface(const Surface::Token::Handle &token, bool exclusive, bool full, const RectInt &rect);

public:
	SurfaceResource();
	SurfaceResource(Surface::Handle surface);
	virtual ~SurfaceResource();

	void create(int width, int height);
	void assign(Surface::Handle surface);
	void clear();
	void reset();

	void create(const VectorInt &x)
		{ create(x[0], x[1]); }

	int get_width() const
		{ Glib::Threads::Mutex::Lock lock(mutex); return width; }
	int get_height() const
		{ Glib::Threads::Mutex::Lock lock(mutex); return height; }
	VectorInt get_size() const
		{ Glib::Threads::Mutex::Lock lock(mutex); return VectorInt(width, height); }
	bool is_exists() const
		{ Glib::Threads::Mutex::Lock lock(mutex); return width > 0 && height > 0; }
	bool is_blank() const
		{ Glib::Threads::Mutex::Lock lock(mutex); return blank; }
	bool has_surface(const Surface::Token::Handle &token) const
		{ Glib::Threads::Mutex::Lock lock(mutex); return surfaces.count(token); }
	template<typename T>
	bool has_surface() const
		{ return has_surface(T::token.handle()); }
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
