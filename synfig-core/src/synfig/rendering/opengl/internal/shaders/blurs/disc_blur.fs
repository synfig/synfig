/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/blurs/disc_blur.fs
**	\brief Disc Blur Fragment Shader
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

layout (location = 0) out vec4 out_color;

const float preci = 0.0000000001;

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));

    int sx = int(ceil(size.x - preci));
    int sy = int(ceil(size.y - preci));

    float k0x = size.x, k0y = size.y;
    float k1x = size.x + 1, k1y = size.y + 1;

    float kk0x = 1.0 / k0x;
    float kk0y = 1.0 / k0y;
    float kk1x = 1.0 / k1x;
    float kk1y = 1.0 / k1y;

    float pp0x = 0, pp0y = 0;
    float pp1x = 0, pp1y = 0;

    int minx = -sx;
    int maxx = +sx;
    int miny = -sy;
    int maxy = +sy;

    vec4 sum = texelFetch(tex, coord, 0);
    float total_weight = 1;
    for(int y = 0; y <= maxy; y++)
    {
        for(int x = 0; x <= maxx; x++)
        {
            if(x == 0 && y == 0) continue;

            float r1s = pp1x * pp1x + pp1y * pp1y;
            if(r1s >= 1) continue;

            float weight = 0.0;
            float r0s = pp0x * pp0x + pp0y * pp0y;
            if(r0s <= 1.0) weight = 1.0;
            else {
                float rr0 = sqrt(r0s);
                float rr1 = sqrt(r1s);
                float drr = rr0 - rr1;
                if(drr > 0) weight = (rr0 * (1.0 - rr1)) / drr;
            }

            if(x == 0) {
                sum += weight * texelFetch(tex, coord + ivec2(x, y), 0);
                sum += weight * texelFetch(tex, coord + ivec2(x, -y), 0);
                total_weight += 2 * weight;
            } else if(y == 0) {
                sum += weight * texelFetch(tex, coord + ivec2(x, y), 0);
                sum += weight * texelFetch(tex, coord + ivec2(-x, y), 0);
                total_weight += 2 * weight;
            } else {
                sum += weight * texelFetch(tex, coord + ivec2(x, y), 0);
                sum += weight * texelFetch(tex, coord + ivec2(-x, y), 0);
                sum += weight * texelFetch(tex, coord + ivec2(x, -y), 0);
                sum += weight * texelFetch(tex, coord + ivec2(-x, -y), 0);
                total_weight += 4 * weight;
            }
            pp0x += kk0x;
            pp1x += kk1x;
        }
        pp0x = 0; pp0y += kk0y;
        pp1x = 0; pp1y += kk1y;
    }
    sum /= total_weight;
    // sum /= total_size;

    out_color = sum;
}
