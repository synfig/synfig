/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/framebuffer.h
**	\brief Framebuffer Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_GL_FRAMEBUFFER_H
#define __SYNFIG_RENDERING_GL_FRAMEBUFFER_H

/* === H E A D E R S ======================================================= */
#include "synfig/color/color.h"

#include "headers.h"
#include <cassert>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

class Framebuffer
{
public:
	// remember to call glViewport once to set appropiate viewport settings
	bool from_pixels(int width, int height, const Color* pixels = nullptr);

    bool from_dims(int width, int height);
	
	void use_write(bool clear = true);
	void use_read(int tex);

	void unuse();

	void clear();

	void reset();

	// TODO: move texture data to a PBO before this function is called so that gpu can transfer in background
	const Color* get_pixels() const;

	bool is_valid() const { return valid; }

    int get_w() const { assert(valid); return width; }
    int get_h() const { assert(valid); return height; }

    GLuint get_id() const { assert(valid); return id; }

private:
	GLuint id, texId, activeTexSlot;

	int width, height;

	bool valid = false;
	bool is_writing = false;
	bool is_reading = false;
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
