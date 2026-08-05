/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/luma_key.fs
**	\brief Luma Key Fragment Shader
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
uniform ivec2 offset;

uniform mat5 mat;

layout (location = 0) out vec4 out_color;

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));
	vec4 col = texelFetch(tex, coord + offset, 0);

    vec4 outc = col;
    outc.r = col.r * mat.m00 + col.g * mat.m10 + col.b * mat.m20 + mat.m40;
    outc.g = col.r * mat.m01 + col.g * mat.m11 + col.b * mat.m21 + mat.m41;
    outc.b = col.r * mat.m02 + col.g * mat.m12 + col.b * mat.m22 + mat.m42;

    float w = col.r * mat.m04 + col.g * mat.m14 + col.b * mat.m24;
    outc.a = col.a * w;

	out_color = outc;
}
