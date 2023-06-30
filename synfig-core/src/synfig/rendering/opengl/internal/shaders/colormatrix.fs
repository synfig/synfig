/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/solid.fs
**	\brief Solid Color Fragment Shader
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

#version 330 core

struct mat5
{
	float m00, m01, m02, m03, m04;
	float m10, m11, m12, m13, m14;
	float m20, m21, m22, m23, m24;
	float m30, m31, m32, m33, m34;
	float m40, m41, m42, m43, m44;
};

uniform sampler2D tex;
uniform mat5 mat;

layout(location = 0) out vec4 out_color;

void main()
{
}
