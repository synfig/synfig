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

    enum CurveType {
        SERPENTINE,
        CUSP,
        LOOP,
        QUADRATIC,
        LINE,
        POINT
    };

    int CrossDir(Vector p0, Vector p1, Vector p2) const
    {
        Vector a = p1 - p0, b = p2 - p1;
        float z = a[0] * b[1] - a[1] * b[0];
        return z < 0 ? -1 : 1;
    }

    Vector3 Cross(Vector3 a, Vector3 b) const
    {
        float cx = a[1] * b[2] - a[2] * b[1];
        float cy = a[2] * b[0] - a[0] * b[2];
        float cz = a[0] * b[1] - a[1] * b[0];
        return Vector3(cx, cy, cz);
    }

    bool ApproxZero(float a) const
    {
        return std::abs(a) < std::numeric_limits<float>::epsilon();
    }

    template<typename T>
    std::vector<std::vector<T>> MultiplyMatrices(const std::vector<std::vector<T>>& mat1,
            const std::vector<std::vector<T>>& mat2) const {
        int rows1 = mat1.size();
        int cols1 = mat1[0].size();
        int cols2 = mat2[0].size();

        std::vector<std::vector<T>> result(rows1, std::vector<T>(cols2, 0.0));

        if (cols1 != mat2.size()) {
            std::cerr << "Matrix multiplication not possible. Inner dimensions must match." << std::endl;
            return result;
        }

        for (int i = 0; i < rows1; ++i) {
            for (int j = 0; j < cols2; ++j) {
                for (int k = 0; k < cols1; ++k) {
                    result[i][j] += mat1[i][k] * mat2[k][j];
                }
            }
        }

        return result;
    }

    CurveType GetCurveType(Vector p0, Vector p1, Vector p2, Vector p3) const
    {
        auto B = MultiplyMatrices(
            std::vector<std::vector<double>>({
                {1, 0, 0, 0},
                {1, 1.0 / 3.0, 0, 0},
                {1, 2.0 / 3.0, 1.0 / 3.0, 0},
                {1, 1, 1, 1},
            }),
            std::vector<std::vector<double>>({
                {p0[0], p0[1], 1},
                {p1[0], p1[1], 1},
                {p2[0], p2[1], 1},
                {p3[0], p3[1], 1},
            })
        );

        Vector3 b0(B[0][0], B[0][1], B[0][2]);
        Vector3 b1(B[1][0], B[1][1], B[1][2]);
        Vector3 b2(B[2][0], B[2][1], B[2][2]);
        Vector3 b3(B[3][0], B[3][1], B[3][2]);

        float d0 = Matrix3(b3, b2, b1).det();
        float d1 = -Matrix3(b3, b2, b0).det();
        float d2 = Matrix3(b3, b1, b0).det();
        float d3 = -Matrix3(b2, b1, b0).det();

        float dI = d1 * d1 * (3 * d2 * d2 - 4 * d3 * d1);

        info("dI: %f", dI);

        if(ApproxZero(dI))
        {
            return CurveType::CUSP;
        }

        if(dI < 0) return CurveType::LOOP;

        return CurveType::SERPENTINE;
    }

	virtual bool run(RunParams&) const
	{
		if(!is_valid()) return true;

		LockWrite ldst(this);
		if(!ldst) return false;

		gl::Context::Lock lock(env().get_or_create_context());

		glEnable(GL_SCISSOR_TEST);

		glViewport(0, 0, ldst->get_width(), ldst->get_height());
		glScissor(target_rect.minx, target_rect.miny, target_rect.get_width(), target_rect.get_height());

        gl::Framebuffer& framebuffer = ldst->get_framebuffer();

        framebuffer.use_write(true);
        glViewport(0, 0, ldst->get_width(), ldst->get_height());

        Contour::ChunkList chunks = contour->get_chunks();

        // Vector tp0;
        // for(int i = 0; i < chunks.size(); i++)
        // {
        //     std::string s;
        //     switch(chunks[i].type)
        //     {
        //         case Contour::CLOSE:
        //             s = "CLOSE: ";
        //             break;
        //         case Contour::MOVE:
        //             s = "MOVE: ";
        //             break;
        //         case Contour::LINE:
        //             s = "LINE: ";
        //             break;
        //         case Contour::CONIC:
        //             s = "CONIC: ";
        //             break;
        //         case Contour::CUBIC:
        //             s = "CUBIC: ";
        //             break;
        //     }
        //     s = s + "(" + std::to_string(tp0[0]) + ":" + std::to_string(tp0[1]) + "):(" + std::to_string(chunks[i].pp0[0]) + ":" + std::to_string(chunks[i].pp0[1]) + ") -> ";
        //     s = s + "(" + std::to_string(chunks[i].p1[0]) + ":" + std::to_string(chunks[i].p1[1]) + "):(" + std::to_string(chunks[i].pp1[0]) + ":" + std::to_string(chunks[i].pp1[1]) + ")";
        //
        //     tp0 = chunks[0].p1;
        //     info(s);
        // }

		Vector ppu = get_pixels_per_unit();

		Matrix bounds_transfromation;
		bounds_transfromation.m00 = ppu[0];
		bounds_transfromation.m11 = ppu[1];
		bounds_transfromation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
		bounds_transfromation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

		Matrix matrix = bounds_transfromation * transformation->matrix;

        Vector p0 = matrix.get_transformed(chunks[0].p1), p3 = matrix.get_transformed(chunks[1].p1);
        Vector p1 = matrix.get_transformed(chunks[1].pp0), p2 = matrix.get_transformed(chunks[1].pp1);

        CurveType curveType = GetCurveType(p0, p1, p2, p3);
        info("CurveType: %d", curveType);

        p0[0] = -(1.0f - 2.0f * (p0[0] / ldst->get_width()));
        p1[0] = -(1.0f - 2.0f * (p1[0] / ldst->get_width()));
        p2[0] = -(1.0f - 2.0f * (p2[0] / ldst->get_width()));
        p3[0] = -(1.0f - 2.0f * (p3[0] / ldst->get_width()));

        p0[1] = -(1.0f - 2.0f * (p0[1] / ldst->get_height()));
        p1[1] = -(1.0f - 2.0f * (p1[1] / ldst->get_height()));
        p2[1] = -(1.0f - 2.0f * (p2[1] / ldst->get_height()));
        p3[1] = -(1.0f - 2.0f * (p3[1] / ldst->get_height()));


        bool convex = true;
        if(CrossDir(p0, p1, p2) != CrossDir(p1, p2, p3) || CrossDir(p0, p1, p2) != CrossDir(p2, p3, p0))
        {
            convex = false;
        }

        std::vector<float> vertices({
            float(p0[0]), float(p0[1]),
            float(p1[0]), float(p1[1]),
            float(p2[0]), float(p2[1]),
            float(p3[0]), float(p3[1])
        });

        std::vector<int> indices;

        if(convex)
        {
            indices = {
                0, 1, 2,
                0, 2, 3
            };
        } else
        {
            indices = {
                0, 1, 2,
                0, 2, 3,
                0, 1, 3
            };
        }

        gl::Programs::Program shader = env().get_or_create_context().get_program("solid");
        shader.use();
        shader.set_color("color", Color::cyan());

        unsigned int VAO, VBO, EBO;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                vertices.size() * sizeof(float),
                vertices.data(),
                GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                indices.size() * sizeof(int),
                indices.data(),
                GL_STATIC_DRAW);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

		glDisable(GL_SCISSOR_TEST);

        framebuffer.unuse();
		return true;
	}
};


Task::Token TaskContourGL::token(
	DescReal<TaskContourGL, TaskContour>("TaskContourGL") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
