/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/blit_gamma.fs
**	\brief Blit Gamma Fragment Shader
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

uniform sampler2D tex;
uniform ivec2 offset;

uniform vec3 gamma;

layout (location = 0) out vec4 out_color;

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));
	vec4 col = texelFetch(tex, coord + offset, 0);
    col.r = col.r < 0 ? -pow(-col.r, gamma.r) : pow(col.r, gamma.r);
    col.g = col.g < 0 ? -pow(-col.g, gamma.g) : pow(col.g, gamma.g);
    col.b = col.b < 0 ? -pow(-col.b, gamma.b) : pow(col.b, gamma.b);
	out_color = col;
}
