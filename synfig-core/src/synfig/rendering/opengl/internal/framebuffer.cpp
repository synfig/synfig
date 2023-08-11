/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/framebuffer.cpp
**	\brief Framebuffer
**
**	\legal
**	......... ... 2023 Bharat Sahlot
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

#include "headers.h"
#include "framebuffer.h"
#include "synfig/general.h"
#include <cassert>
#include <vector>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
bool
gl::Framebuffer::from_pixels(int width, int height, const Color* pixels)
{
	valid = false;

	this->width = width;
	this->height = height;

	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); 

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;

	valid = true;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool
gl::Framebuffer::from_dims(int width, int height)
{
    return from_pixels(width, height);
}

void
gl::Framebuffer::use_write(bool clear)
{
	assert(is_valid());

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	if(clear)
	{
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	is_writing = true;
}

void
gl::Framebuffer::use_read(int tex)
{
	assert(is_valid());

	if(is_writing)
	{
		warning("Opengl[M] -> Attempting to read from a framebuffer while it is used for writing: %d", id);
	}

	// TODO: confirm this works
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, texId);
	is_reading = true;
	activeTexSlot = tex;
}

void
gl::Framebuffer::unuse()
{
	assert(is_valid());

	// TODO: potential bug where glActiveTexture is called somewhere else and we still unbind in unuse, which unbinds some other texture
	if(is_writing)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if(is_reading)
	{
		glActiveTexture(GL_TEXTURE0 + activeTexSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	is_writing = is_reading = false;
}

void
gl::Framebuffer::clear()
{
	assert(is_valid());

	use_write();
	unuse();
}

void
gl::Framebuffer::reset()
{
	if(valid)
	{
		if(is_writing || is_reading)
		{
			warning("Opengl[W] -> Attempting to reset a framebuffer which is currently used for reading or writing");
		}

		glDeleteTextures(1, &texId);
		glDeleteFramebuffers(1, &id);
	}
	valid = false;
}

const Color*
gl::Framebuffer::get_pixels() const
{
	assert(is_valid());

	Color* color = new Color[width * height];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, color);
	glBindTexture(GL_TEXTURE_2D, 0);

	return color;
}

/* === E N T R Y P O I N T ================================================= */
