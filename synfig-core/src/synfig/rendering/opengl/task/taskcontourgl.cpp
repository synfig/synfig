/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskcontourgl.cpp
**	\brief TaskContourGL
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

#include "synfig/color/color.h"
#include "synfig/matrix.h"
#include "synfig/vector.h"
#include <vector>
#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "taskgl.h"

#include "synfig/general.h"
#include "../surfacegl.h"
#include "../internal/headers.h"

#include "../internal/context.h"
#include "../internal/environment.h"
#include "../internal/shaders.h"
#include "../internal/plane.h"

#include "../../common/task/taskcontour.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskContourGL: public TaskContour, public TaskGL
{
public:
	typedef etl::handle<TaskContourGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams&) const
	{
		if(!is_valid()) return true;

		LockWrite ldst(this);
		if(!ldst) return false;

		gl::Context::Lock lock(env().get_or_create_context());

		glEnable(GL_SCISSOR_TEST);

		glViewport(0, 0, ldst->get_width(), ldst->get_height());
		glScissor(target_rect.minx, target_rect.miny, target_rect.get_width(), target_rect.get_height());

        gl::Framebuffer framebuffer;
        framebuffer.from_dims(ldst->get_width(), ldst->get_height(), true);

        framebuffer.use_write(true);
        glViewport(0, 0, ldst->get_width(), ldst->get_height());

        Contour::ChunkList chunks = contour->get_chunks();

		Vector ppu = get_pixels_per_unit();

		Matrix bounds_transfromation;
		bounds_transfromation.m00 = ppu[0];
		bounds_transfromation.m11 = ppu[1];
		bounds_transfromation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
		bounds_transfromation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

		Matrix matrix = bounds_transfromation * transformation->matrix;

        std::vector<GLubyte> pathCommands;
        std::vector<float> pathCoords;

        for(int i = 0; i < chunks.size(); i++)
        {
            Vector p0 = matrix.get_transformed(chunks[i].p1);
            Vector p1 = matrix.get_transformed(chunks[i].pp0);
            Vector p2 = matrix.get_transformed(chunks[i].pp1);

            p0[0] = -(1.0f - 2.0f * (p0[0] / ldst->get_width()));
            p1[0] = -(1.0f - 2.0f * (p1[0] / ldst->get_width()));
            p2[0] = -(1.0f - 2.0f * (p2[0] / ldst->get_width()));

            p0[1] = -(1.0f - 2.0f * (p0[1] / ldst->get_height()));
            p1[1] = -(1.0f - 2.0f * (p1[1] / ldst->get_height()));
            p2[1] = -(1.0f - 2.0f * (p2[1] / ldst->get_height()));

            switch (chunks[i].type) {
                case Contour::ChunkType::MOVE:
                    pathCommands.push_back(GL_MOVE_TO_NV);
                    pathCoords.push_back((p0[0]));
                    pathCoords.push_back((p0[1]));
                    break;
                case Contour::ChunkType::LINE:
                    pathCommands.push_back(GL_LINE_TO_NV);
                    pathCoords.push_back((p0[0]));
                    pathCoords.push_back((p0[1]));
                    break;
                case Contour::ChunkType::CONIC:
                    pathCommands.push_back(GL_QUADRATIC_CURVE_TO_NV);

                    pathCoords.push_back((p1[0]));
                    pathCoords.push_back((p1[1]));

                    pathCoords.push_back((p0[0]));
                    pathCoords.push_back((p0[1]));
                    break;
                case Contour::ChunkType::CUBIC:
                    pathCommands.push_back(GL_CUBIC_CURVE_TO_NV);
                    pathCoords.push_back((p1[0]));
                    pathCoords.push_back((p1[1]));

                    pathCoords.push_back((p2[0]));
                    pathCoords.push_back((p2[1]));

                    pathCoords.push_back((p0[0]));
                    pathCoords.push_back((p0[1]));
                    break;
                case Contour::ChunkType::CLOSE:
                    pathCommands.push_back(GL_CLOSE_PATH_NV);
                    break;
            }
        }

        unsigned int pathObj = glGenPathsNV(1);

        glPathCommandsNV(pathObj, pathCommands.size(), pathCommands.data(),
                pathCoords.size(), GL_FLOAT, pathCoords.data());

        glStencilFillPathNV(pathObj, GL_COUNT_UP_NV, 0x1F);

        glEnable(GL_STENCIL_TEST);
        if(contour->winding_style == Contour::WindingStyle::WINDING_NON_ZERO) glStencilFunc(GL_NOTEQUAL, 0, 0x1F);
        else glStencilFunc(GL_NOTEQUAL, 0, 0x1);

        glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

        gl::Programs::Program shader = env().get_or_create_context().get_program("solid");
        shader.use();
        shader.set_color("color", contour->color);

        gl::Plane plane;
        plane.render();

        framebuffer.unuse();
        glDisable(GL_STENCIL_TEST);

        gl::Framebuffer& dest = ldst->get_framebuffer();
        dest.use_write(true);
        framebuffer.use_read(0);

        shader = env().get_or_create_context().get_program("blit");
        shader.use();
        shader.set_1i("tex", 0);
        shader.set_2i("offset", VectorInt(0, 0));

        plane.render();

        framebuffer.unuse();
        framebuffer.reset();

        dest.unuse();

        glDeletePathsNV(pathObj, 1);

		glDisable(GL_SCISSOR_TEST);

		return true;
	}
};


// Task::Token TaskContourGL::token(
// 	DescReal<TaskContourGL, TaskContour>("TaskContourGL") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
