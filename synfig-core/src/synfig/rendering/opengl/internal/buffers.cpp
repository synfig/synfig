/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/buffers.cpp
**	\brief Buffers
**
**	$Id$
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cmath>
#include <cstring>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/vector.h>

#include "buffers.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::Buffers::Buffer::Buffer(Context &context, GLenum target, GLenum usage, int size):
	context(context), target(target), usage(usage), id(), size(size), first(), last()
{
	Context::Lock lock(context);
	glGenBuffers(1, &id);
	glBindBuffer(target, id);
	glBufferData(target, size, NULL, usage);
	glBindBuffer(target, 0);
}

gl::Buffers::Buffer::~Buffer() {
	Context::Lock lock(context);
	assert(is_free());
	glBindBuffer(target, 0);
	glDeleteBuffers(1, &id);
}


gl::Buffers::Buffer::Chunk*
gl::Buffers::Buffer::alloc(int size)
{
	// find empty chunk
	Chunk *curr = NULL;
	Chunk *next = first;
	int offset = 0;
	while(true) {
		offset = curr ? curr->offset + curr->size : 0;
		int free_size = (next ? next->offset : this->size) - offset;
		if (size <= free_size)
			return new Chunk(*this, curr, offset, size);
		if (!next) break;
		curr = next;
		next = next->next;
	};

	{ // resize
		Context::Lock lock(context);
		int new_size = (int)ceil(1.5*(offset + size));
		std::vector<char> data(new_size);
		glBindBuffer(target, id);
		glGetBufferSubData(target, 0, this->size, &data.front());
		this->size = new_size;
		glBufferData(target, this->size, &data.front(), usage);
		glBindBuffer(target, 0);
	}

	return new Chunk(*this, last, offset, size);
}



gl::Buffers::Buffers(Context &context):
	context(context),
	array_buffer(context, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, 1024*1024),
	element_array_buffer(context, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, 1024*1024)
{
	Context::Lock lock(context);

	Vector default_quad_buffer_data[] = {
		Vector(-1.0, -1.0),
		Vector(-1.0,  1.0),
		Vector( 1.0, -1.0),
		Vector( 1.0,  1.0) };
	default_quad_buffer = get_array_buffer(default_quad_buffer_data);
}

gl::Buffers::~Buffers()
{
	Context::Lock lock(context);

	default_quad_buffer = BufferLock();

	// delete vertex arrays
	for(std::list<VertexArray>::iterator i = vertex_arrays.begin(); i != vertex_arrays.end(); ++i)
	{
		assert(!i->locks);
		glDeleteVertexArrays(1, &i->id);
	}
}

gl::Buffers::BufferLock
gl::Buffers::get_buffer(Buffer &buffer, const void *data, int size)
{
	if (!data) return BufferLock();
	assert(size >= 0);
	if (!size) return BufferLock();

	Context::Lock lock(context);
	Buffer::Chunk *chunk = buffer.alloc(size);
	glBindBuffer(buffer.target, buffer.id);
	glBufferSubData(buffer.target, chunk->offset, size, data);
	glBindBuffer(buffer.target, 0);
	return BufferLock(chunk);
}

gl::Buffers::VertexArrayLock
gl::Buffers::get_vertex_array()
{
	Context::Lock lock(context);
	for(std::list<VertexArray>::iterator i = vertex_arrays.begin(); i != vertex_arrays.end(); ++i)
		if (!i->locks)
			return VertexArrayLock(&*i);
	vertex_arrays.push_back(VertexArray());
	glGenVertexArrays(1, &vertex_arrays.back().id);
	return VertexArrayLock(&vertex_arrays.back());
}

/* === E N T R Y P O I N T ================================================= */
