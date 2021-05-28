/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/context.h
**	\brief Context Header
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

#ifndef __SYNFIG_RENDERING_GL_CONTEXT_H
#define __SYNFIG_RENDERING_GL_CONTEXT_H

/* === H E A D E R S ======================================================= */

#include <cassert>

#include <vector>
#include <utility>

#include <mutex>

#include <GL/gl.h>
#include <GL/glx.h>

#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

class Context
{
public:
	class Lock {
	private:
		Context &context;
	public:
		Lock(Context &context): context(context) { context.use(); }
		Lock(Lock &other): context(other.context) { context.use(); }
		~Lock() { context.unuse(); }

		Context& get() const { return context; }
		Context* operator-> () const { return &get(); }
	};

	struct ContextInfo {
		Display *display;
		GLXDrawable drawable;
		GLXDrawable read_drawable;
		GLXContext context;

		ContextInfo():
			display(NULL),
			drawable(None),
			read_drawable(None),
			context(NULL) { }

		bool operator== (const ContextInfo &other) const
		{
			return display == other.display
				&& drawable == other.drawable
				&& read_drawable == other.read_drawable
				&& context == other.context;
		}

		bool operator!= (const ContextInfo &other) const
		{
			return !(*this == other);
		}

		void make_current() const;
		static ContextInfo get_current(Display *default_display);
	};

private:
	std::recursive_mutex rec_mutex;

	Display *display;
	GLXFBConfig config;
	GLXPbuffer pbuffer;
	GLXContext context;

	ContextInfo context_info;
	std::vector<ContextInfo> context_stack;

	static std::pair<GLenum, const char*> enum_strings[];

public:
	Context();
	~Context();

	bool is_valid() const { return context; }
	bool is_current() const;
	void use();
	void unuse();

	void check(const char *s = "");
	const char* get_enum_string(GLenum x);
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
