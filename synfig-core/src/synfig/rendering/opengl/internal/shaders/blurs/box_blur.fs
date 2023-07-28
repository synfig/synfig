/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/blurs/box_blur.fs
**	\brief Box Blur Fragment Shader
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

uniform ivec2 size;
uniform ivec4 rect;

layout (location = 0) out vec4 out_color;

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));

    int minx = max(coord.x - size.x, rect[0]);
    int maxx = min(coord.x + size.x, rect[1] - 1);
    int miny = max(coord.y - size.y, rect[2]);
    int maxy = min(coord.y + size.y, rect[3] - 1);

    int total_size = 0;

    vec4 sum = vec4(0);
    for(int y = miny; y <= maxy; y++)
    {
        for(int x = minx; x <= maxx; x++)
        {
            sum += texelFetch(tex, ivec2(x, y), 0);
            total_size++;
        }
    }
    sum /= total_size;

    out_color = sum;
}
