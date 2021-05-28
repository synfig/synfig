/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/buffers.h
**	\brief Buffers Header
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_GL_BUFFERS_H
#define __SYNFIG_RENDERING_GL_BUFFERS_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>

#include "context.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

class Buffers
{
public:
	class Buffer {
	public:
		class Chunk;

		Context &context;
		GLenum target;
		GLenum usage;
		GLuint id;
		int size;
		Chunk *first, *last;

		class Chunk {
		public:
			const int offset;
			const int size;
			Buffer &buffer;
			Chunk *prev, *next;
			int locks;

			Chunk(Buffer &buffer, Chunk *prev, int offset, int size):
				offset(offset), size(size), buffer(buffer), prev(prev), next(prev ? prev->next : buffer.first), locks()
			{
				(prev ? prev->next : buffer.first) = this;
				(next ? next->prev : buffer.last) = this;
			}

			~Chunk()
			{
				(prev ? prev->next : buffer.first) = next;
				(next ? next->prev : buffer.last) = prev;
			}
		};

		Buffer(Context &context, GLenum target, GLenum usage, int size);
		~Buffer();
		Chunk* alloc(int size);
		bool is_free() const { return !first; }
	};

	class BufferLock {
	private:
		friend class Buffers;
		Buffer::Chunk *chunk;

		void set(Buffer::Chunk *chunk) {
			if (this->chunk) {
				--this->chunk->locks;
				if (!this->chunk->locks)
					delete this->chunk;
			}
			this->chunk = chunk;
			if (this->chunk) ++this->chunk->locks;
		}

		BufferLock(Buffer::Chunk *chunk): chunk()
			{ set(chunk); }
	public:
		BufferLock(): chunk()
			{ }
		BufferLock(const BufferLock &other): chunk()
			{ *this = other; }
		~BufferLock()
			{ set(NULL); }
		BufferLock& operator = (const BufferLock &other)
			{ set(other.chunk); return *this; }
		GLuint get_id() const
			{ return chunk ? chunk->buffer.id : 0; }
		int get_offset() const
			{ return chunk ? chunk->offset : 0; }
		int get_size() const
			{ return chunk ? chunk->size : 0; }
		const void* get_pointer() const
			{ return (const void*)(ptrdiff_t)get_offset(); }
	};

	class VertexArray {
	public:
		GLuint id;
		int locks;
		VertexArray(): id(), locks() { }
	};

	class VertexArrayLock {
	private:
		friend class Buffers;
		VertexArray *vertex_array;

		void set(VertexArray *vertex_buffer) {
			if (this->vertex_array) --this->vertex_array->locks;
			this->vertex_array = vertex_buffer;
			if (this->vertex_array) ++this->vertex_array->locks;
		}

		VertexArrayLock(VertexArray *vertex_buffer): vertex_array()
			{ set(vertex_buffer); }
	public:
		VertexArrayLock(): vertex_array()
			{ }
		VertexArrayLock(const VertexArrayLock &other): vertex_array()
			{ *this = other; }
		~VertexArrayLock()
			{ set(NULL); }
		VertexArrayLock& operator = (const VertexArrayLock &other)
			{ set(other.vertex_array); return *this; }
		GLuint get_id() const
			{ return vertex_array ? vertex_array->id : 0; }
	};

public:
	Context &context;

private:
	Buffer array_buffer;
	Buffer element_array_buffer;
	std::list<VertexArray> vertex_arrays;

	BufferLock default_quad_buffer;

public:
	Buffers(Context &context);
	~Buffers();

private:
	BufferLock get_buffer(Buffer &buffer, const void *data, int size);

	template<typename T>
	BufferLock get_buffer(Buffer &buffer, const std::vector<T> &data, int offset = 0, int count = 0)
	{
		assert(offset >= 0);
		assert(count >= 0);
		if (!count) count = (int)data.size() - offset;
		assert(offset + count <= (int)data.size());
		assert(count >= 0);
		if (!count) return BufferLock();
		return get_buffer(buffer, &data.front(), count*sizeof(data.front()));
	}

public:
	// returns buffer with four synfig::Vector: (-1, -1), (-1, 1), (1, -1), (1, 1)
	BufferLock get_default_quad_buffer()
		{ return default_quad_buffer; }

	BufferLock get_array_buffer(const void *data, int size)
		{ return get_buffer(array_buffer, data, size); }

	template<typename T>
	BufferLock get_array_buffer(const std::vector<T> &data, int offset = 0, int count = 0)
		{ return get_buffer(array_buffer, data, offset, count); }

	template<typename T, int size>
	BufferLock get_array_buffer(const T (&data)[size], int offset = 0, int count = 0)
		{ return get_buffer(array_buffer, &data[offset], count ? sizeof(T)*count : sizeof(data)); }

	BufferLock get_element_array_buffer(const void *data, int size)
		{ return get_buffer(element_array_buffer, data, size); }

	template<typename T>
	BufferLock get_element_array_buffer(const std::vector<T> &data, int offset = 0, int count = 0)
		{ return get_buffer(element_array_buffer, data, offset, count); }

	template<typename T, int size>
	BufferLock get_element_array_buffer(const T (&data)[size], int offset = 0, int count = 0)
		{ return get_buffer(element_array_buffer, &data[offset], count ? sizeof(T)*count : sizeof(data)); }

	VertexArrayLock get_vertex_array();
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
