/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/surface.h
**	\brief Surface Header
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_SURFACE_H
#define __SYNFIG_RENDERING_SURFACE_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <vector>

#include <mutex>
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

	class DescBase {
	protected:
		template<typename T>
		static Surface* fabric_template()
			{ return new T(); }
	public:
		const String name;
		const Fabric fabric;

		DescBase(const String &name, Fabric fabric):
			name(name),
			fabric(fabric)
		{ assert(fabric); }
	};

	class Token: public synfig::Token, public DescBase {
	public:
		typedef ConstRef<Token> Handle;

	private:
		template<typename T>
		T* fabric_template()
			{ return new T(); }

	public:
		Token(const DescBase &desc):
			synfig::Token(token.handle()),
			DescBase(desc) { }

		inline Handle handle() const
			{ return Handle(*this); }
	};

	template<typename Type>
	class Desc: public DescBase {
	public:
		Desc(const String &name):
			DescBase(
				name,
				&DescBase::fabric_template<Type> )
		{ }
	};


	static synfig::Token token;
	virtual Token::Handle get_token() const = 0;

private:
	bool blank;
	int width;
	int height;

protected:
	void set_desc(int width, int height, bool blank);

	virtual bool create_vfunc(int /* width */, int /* height */)
		{ return false; }
	virtual bool assign_vfunc(const Surface & /* other */)
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

	bool compare_with(synfig::rendering::Surface::Handle s) const;

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

	template<typename TypeSurface, bool write, bool exclusive>
	class LockBase {
	public:
		const Handle resource;
		const bool full;
		const RectInt rect;

	private:
		bool lock_token;
		Surface::Token::Handle token;
		Surface::Handle surface;

		void lock() {
			if (resource) {
				if (write) resource->rwlock.writer_lock();
				      else resource->rwlock.reader_lock();
			}
		}
		void unlock() {
			if (resource) {
				surface.reset();
				if (write) resource->rwlock.writer_unlock();
				      else resource->rwlock.reader_unlock();
			}
		}

		LockBase(const LockBase&): full(), lock_token() { }

	public:
		explicit LockBase(const Handle &resource):
			resource(resource), full(true), lock_token(false)
			{ lock(); }
		LockBase(const Handle &resource, const RectInt &rect):
			resource(resource), full(false), rect(rect), lock_token(false)
			{ lock(); }
		LockBase(const Handle &resource, const Surface::Token::Handle &token):
			resource(resource), full(true), lock_token(true), token(token)
			{ lock(); }
		LockBase(const Handle &resource, const RectInt &rect, const Surface::Token::Handle &token):
			resource(resource), full(false), rect(rect), lock_token(true), token(token)
			{ lock(); }
		~LockBase() { unlock(); }

		bool convert(const Surface::Token::Handle &token, bool create = true, bool any = false) {
			if (!resource) return false;
			if (lock_token && token != this->token) return false;
			return surface = resource->get_surface(token, exclusive, full, rect, create, any);
		}

		template<typename T>
		bool convert(bool create = true, bool any = false)
			{ return convert(T::token.handle(), create, any); }

		bool is_lock_tocken() const
			{ return lock_token; }
		Surface::Token::Handle get_lock_token() const
			{ return token; }
		Surface::Token::Handle get_token() const
			{ return surface ? surface->get_token() : Surface::Token::Handle(); }
		const Handle& get_resource() const
			{ return resource; }
		const Surface::Handle& get_handle() const
			{ return surface; }
		template<typename T>
		etl::handle<T> cast() const
			{ return etl::handle<T>::cast_dynamic(surface); }
		TypeSurface* get_surface() const
			{ return surface.get(); }
		operator bool() const
			{ return surface; }
	};

	typedef LockBase<const Surface, false, false> LockReadBase;
	typedef LockBase<Surface, true, true> LockWriteBase;
	typedef LockBase<Surface, false, true> SemiLockWriteBase; //!< helps to avoid crashes but may cause visual artifacts

	template<typename T>
	class LockRead: public LockReadBase {
	public:
		typedef T Type;
		explicit LockRead(const Handle &resource):
			LockReadBase(resource, Type::token.handle())
			{ convert(get_lock_token()); }
		LockRead(const Handle &resource, const RectInt &rect):
			LockReadBase(resource, rect, Type::token.handle())
			{ convert(get_lock_token()); }
		LockRead(const Handle &resource, const Surface::Token::Handle &token):
			LockReadBase(resource, token)
			{ convert(get_lock_token()); }
		LockRead(const Handle &resource, const RectInt &rect, const Surface::Token::Handle &token):
			LockReadBase(resource, rect, token)
			{ convert(get_lock_token()); }
		etl::handle<Type> cast_handle() const
			{ return cast<Type>(); }
		const Type* get() const
			{ return dynamic_cast<Type*>(get_handle().get()); }
		const Type* operator->() const
			{ assert(get()); return get(); }
		const Type& operator*() const
			{ assert(get()); return *get(); }
	};

	template<typename T>
	class LockWrite: public LockWriteBase {
	public:
		typedef T Type;
		explicit LockWrite(const Handle &resource):
			LockWriteBase(resource, Type::token.handle())
			{ convert(get_lock_token()); }
		LockWrite(const Handle &resource, const RectInt &rect):
			LockWriteBase(resource, rect, Type::token.handle())
			{ convert(get_lock_token()); }
		LockWrite(const Handle &resource, const Surface::Token::Handle &token):
			LockWriteBase(resource, token)
			{ convert(get_lock_token()); }
		LockWrite(const Handle &resource, const RectInt &rect, const Surface::Token::Handle &token):
			LockWriteBase(resource, rect, token)
			{ convert(get_lock_token()); }
		etl::handle<Type> cast_handle() const
			{ return cast<Type>(); }
		Type* get() const
			{ return dynamic_cast<Type*>(get_handle().get()); }
		Type* operator->() const
			{ assert(get()); return get(); }
		Type& operator*() const
			{ assert(get()); return *get(); }
	};

	template<typename T>
	class SemiLockWrite: public SemiLockWriteBase {
	public:
		typedef T Type;
		explicit SemiLockWrite(const Handle &resource):
			SemiLockWriteBase(resource, Type::token.handle())
			{ convert(get_lock_token()); }
		SemiLockWrite(const Handle &resource, const RectInt &rect):
			SemiLockWriteBase(resource, rect, Type::token.handle())
			{ convert(get_lock_token()); }
		SemiLockWrite(const Handle &resource, const Surface::Token::Handle &token):
			SemiLockWriteBase(resource, token)
			{ convert(get_lock_token()); }
		SemiLockWrite(const Handle &resource, const RectInt &rect, const Surface::Token::Handle &token):
			SemiLockWriteBase(resource, rect, token)
			{ convert(get_lock_token()); }
		etl::handle<Type> cast_handle() const
			{ return cast<Type>(); }
		Type* get() const
			{ return dynamic_cast<Type*>(get_handle().get()); }
		Type* operator->() const
			{ assert(get()); return get(); }
		Type& operator*() const
			{ assert(get()); return *get(); }
	};

private:
	static int last_id;

	int id;
	int width;
	int height;
	bool blank;
	Map surfaces;

	mutable std::mutex mutex;
	mutable Glib::Threads::RWLock rwlock;

	Surface::Handle get_surface(
		const Surface::Token::Handle &token,
		bool exclusive,
		bool full,
		const RectInt &rect,
		bool create,
		bool any );

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

	int get_id() const //!< helps to debug of renderer optimizers
		{ return id; }
	int get_width() const
		{ std::lock_guard<std::mutex> lock(mutex); return width; }
	int get_height() const
		{ std::lock_guard<std::mutex> lock(mutex); return height; }
	VectorInt get_size() const
		{ std::lock_guard<std::mutex> lock(mutex); return VectorInt(width, height); }
	bool is_exists() const
		{ std::lock_guard<std::mutex> lock(mutex); return width > 0 && height > 0; }
	bool is_blank() const
		{ std::lock_guard<std::mutex> lock(mutex); return blank; }
	bool has_surface(const Surface::Token::Handle &token) const
		{ std::lock_guard<std::mutex> lock(mutex); return surfaces.count(token); }
	template<typename T>
	bool has_surface() const
		{ return has_surface(T::token.handle()); }
	bool get_tokens(std::vector<Surface::Token::Handle> &outTokens) const {
		std::lock_guard<std::mutex> lock(mutex);
		for(Map::const_iterator i = surfaces.begin(); i != surfaces.end(); ++i)
			outTokens.push_back(i->first);
		return !surfaces.empty();
	}
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
