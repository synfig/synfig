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

uniform vec2 size;
uniform ivec2 psize;
uniform bool horizontal;

layout (location = 0) out vec4 out_color;

const float PI = 3.1415926535897932384626433832795029;
const float preci = 0.0000000001;
const float K = 1.0 / sqrt(2.0 * PI);

float gauss(int x, float r)
{
    r += 0.5;
    return K * exp((-0.5 * x * x) / (r * r)) / r;
}

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));

    int sx = psize.x;
    int sy = psize.y;

    if(horizontal) {
        int minx = -sx;
        int maxx = +sx;

        float total = gauss(0, size.x);
        vec4 sum = texelFetch(tex, coord, 0) * total;
        for(int x = 1; x <= maxx; x++)
        {
            float weight = gauss(x, size.x);
            sum += weight * texelFetch(tex, coord + ivec2(x, 0), 0);
            sum += weight * texelFetch(tex, coord + ivec2(-x, 0), 0);
            total += 2 * weight;
        }
        out_color = sum / total;
    } else {
        int miny = -sy;
        int maxy = +sy;

        float total = gauss(0, size.y);
        vec4 sum = texelFetch(tex, coord, 0) * total;
        for(int y = 1; y <= maxy; y++)
        {
            float weight = gauss(y, size.y);
            sum += weight * texelFetch(tex, coord + ivec2(0, y), 0);
            sum += weight * texelFetch(tex, coord + ivec2(0, -y), 0);
            total += 2 * weight;
        }
        out_color = sum / total;
    }
}
