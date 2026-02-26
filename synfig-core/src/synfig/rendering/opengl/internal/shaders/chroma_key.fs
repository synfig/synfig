/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/chrome_key.fs
**	\brief Chroma Key Fragment Shader
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

uniform vec2 key; // u and v key
uniform vec2 bounds; // lower and upper bound
uniform bool desaturate; // lower and upper bound

layout (location = 0) out vec4 out_color;

const mat3 encode_yuv = mat3(
	0.299f, 0.587f, 0.114f,
	-0.168736, -0.331264, 0.5,
	0.5, -0.418688, -0.081312 );

const mat3 decode_yuv = mat3(
	1, 0, 1.402,
	1, -0.344136, -0.714136,
	1, 1.772, 0 );

float get_y(vec4 col)
{
	return col.r * encode_yuv[0][0]
		 + col.g * encode_yuv[0][1]
		 + col.b * encode_yuv[0][2];
}

float get_u(vec4 col)
{
	return col.r * encode_yuv[1][0]
		 + col.g * encode_yuv[1][1]
		 + col.b * encode_yuv[1][2];
}

float get_v(vec4 col)
{
	return col.r * encode_yuv[2][0]
		 + col.g * encode_yuv[2][1]
		 + col.b * encode_yuv[2][2];
}

float get_s(vec4 col)
{
	float u = get_u(col);
	float v = get_v(col);

	return sqrt(u * u + v * v);
}

vec3 from_yuv(float y, float u, float v)
{
	vec3 col = vec3(0);
	col.r = y * decode_yuv[0][0] + u * decode_yuv[0][1] + v * decode_yuv[0][2];
	col.g = y * decode_yuv[1][0] + u * decode_yuv[1][1] + v * decode_yuv[1][2];
	col.b = y * decode_yuv[2][0] + u * decode_yuv[2][1] + v * decode_yuv[2][2];
	return col;
}

vec4 set_s(vec4 col, float x)
{
	float u = get_u(col);
	float v = get_v(col);
	float s = sqrt(u * u + v * v);

    if(s > 0)
    {
        u = (u / s) * x;
        v = (v / s) * x;
        return vec4(from_yuv(get_y(col), u, v), col.a);
    }
    return col;
}

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));
	vec4 col = texelFetch(tex, coord + offset, 0);

    float dist = pow(get_u(col) - key.x, 2) + pow(get_v(col) - key.y, 2);
    if(dist < bounds.x * bounds.x) col.a = 0;
    else if(dist < bounds.y * bounds.y) {
        col.a = (col.a * (sqrt(dist) - bounds.x) / (bounds.y - bounds.x));
        if(desaturate) col = set_s(col, 0);
    }
	out_color = col;
}
