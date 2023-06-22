/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/context.cpp
**	\brief Context
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#include <cassert>

#include "context.h"
#include "headers.h"

#include "synfig/general.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

#define OPENGL_DEBUG_OUTPUT

#ifdef OPENGL_DEBUG_OUTPUT
void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes
	
	const char* sourceStr = "Other";
	const char* typeStr = "Other";

	const int ctx_id = *((int*)userParam);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    }
	
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			error("Opengl[H-%d](%s:%s:%d): %s", ctx_id, sourceStr, typeStr, id, message);
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			warning("Opengl[M-%d](%s:%s:%d): %s", ctx_id, sourceStr, typeStr, id, message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			warning("Opengl[L-%d](%s:%s:%d): %s", ctx_id, sourceStr, typeStr, id, message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			info("Opengl[N-%d](%s:%s:%d): %s", ctx_id, sourceStr, typeStr, id, message);
			break;
	}
}
#endif

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
unsigned int gl::Context::cnt = 0;

gl::Context::Context(gl::Context* par) : id(cnt++)
{
	if(par == nullptr)
	{
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef OPENGL_DEBUG_OUTPUT
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_COCOA_MENUBAR, GL_FALSE);
#endif

		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

		info("Opengl[N-%d]: GLFW Initialized", id);
	}

    glfwWindow = glfwCreateWindow(400, 400, "opengl main hidden window", 
			NULL, 
			par != NULL ? par->glfwWindow : NULL);

    if(glfwWindow == NULL)
    {
        error("Opengl[H-%d]: Failed to create opengl context", id);
		return;
    }

	if(par == nullptr)
	{
		glfwMakeContextCurrent(glfwWindow);
		if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			error("Opengl[H-%d]: Failed to initialize GLAD", id);
			return;
		}
		info("Opengl[N-%d]: GLAD Initialized", id);
	}

#ifdef OPENGL_DEBUG_OUTPUT
	glfwMakeContextCurrent(glfwWindow);
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, &id);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif
}

gl::Context::~Context()
{
	glfwDestroyWindow(glfwWindow);
}

void gl::Context::use()
{
	mutex.lock();
	assert(glfwWindow);
	glfwMakeContextCurrent(glfwWindow);
}

void gl::Context::unuse()
{
	assert(glfwWindow);
	glfwMakeContextCurrent(NULL);
	mutex.unlock();
}

/* === E N T R Y P O I N T ================================================= */
