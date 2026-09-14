/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/downscale.fs
**	\brief Downscale Fragment Shader
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

uniform vec2 pixelSize; // srcDims / destDims
uniform float mul_k; // |destDims| / |srcDims|

layout (location = 0) out vec4 out_color;

const float preci = 0.000001;

vec4 do_row(int x, float kx)
{
    int ys = int(ceil(pixelSize.y * coord.y));
    int ye = int(floor(pixelSize.y * (coord.y + 1)));

    for(int y = ys; y <= ye; y++)
    {
        col += texelFetch(tex, vec2(x, y) + offset, 0) * kx;
    }

    // partial first col
    float ky = float(ys) - (pixelSize.y * coord.y);
    if(ky > 0) col += texelFetch(tex, vec2(x, floor(pixelSize.y * coord.y)), 0) * ky * kx;

    // partial last col
    ky = (pixelSize.y * (coord.y + 1)) - ye;
    if(ky > 0) col += texelFetch(tex, vec2(x, floor(pixelSize.y * (coord.y + 1))), 0) * ky * kx;
}

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));
    vec4 col = vec4(0);

    int xs = int(ceil(pixelSize.x * coord.x));
    int xe = int(floor(pixelSize.x * (coord.x + 1)));

    // full row
    for(int x = xs; x <= xe; x++)
    {
        do_row(x, 1);
    }

    // partial first row
    {
        float pos = pixelSize.x * coord.x;
        float kx = float(xs) - pos;

        if(kx > 0)
        {
            do_row(floor(pixelSize.x * coord.x), kx);
        }
    }

    // partial last row
    {
        float pos = pixelSize.x * (coord.x + 1);
        float kx = pos - float(xe);

        if(kx > 0)
        {
            do_row(floor(pixelSize.x * (coord.x + 1)), kx);
        }
    }

    out_color = col * mul_k;
}
