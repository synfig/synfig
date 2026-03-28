/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/blend.fs
**	\brief Color Blend Shader
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
#define method_#0

uniform bool use_a;
uniform bool use_b;

uniform float amount;
uniform sampler2D sampler_a;
uniform sampler2D sampler_b;
uniform ivec2 offset_a;
uniform ivec2 offset_b;
layout(location = 0) out vec4 out_color;

const float epsilon = 0.000001;

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

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 blend_composite(vec4 src, vec4 dest, float amount)
{
	float a_src = src.a * amount;
	float a_dest = dest.a;

	src *= a_src;
	dest *= a_dest;

	dest = src + dest * (1 - a_src);

	a_dest = a_src + a_dest * (1 - a_src);

	if(abs(a_dest) > epsilon)
	{
		dest /= a_dest;
		dest.a = a_dest;
	}
	else
	{
		dest = vec4(0);
	}
    return dest;
}

vec4 blend_straight(vec4 src, vec4 dest, float amount)
{
    float a_out = (src.a - dest.a) * amount + dest.a;
	if (abs(a_out) > epsilon)
    {
		dest = ((src * src.a - dest * dest.a) * amount + dest * dest.a) / a_out;
        dest.a = a_out;
    }
	else
    {
		dest = vec4(0);
    }

    return dest;
}

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord));

	vec4 src = vec4(0, 0, 0, 0);
	if(use_a) src = texelFetch(sampler_a, coord + offset_a, 0);

	vec4 dest = vec4(0, 0, 0, 0);
	if(use_b) dest = texelFetch(sampler_b, coord + offset_b, 0);

#ifdef method_composite

    dest = blend_composite(src, dest, amount);

#endif
#ifdef method_behind

	src.a *= amount;

	vec4 temp = dest;
	dest = src;
	src = temp;

	float a_src = src.a;
	float a_dest = dest.a;

	src *= a_src;
	dest *= a_dest;

	dest = src + dest * (1 - a_src);

	a_dest = a_src + a_dest * (1 - a_src);

	if(abs(a_dest) > epsilon)
	{
		dest /= a_dest;
		dest.a = a_dest;
	}
	else
	{
		dest = vec4(0);
	}
#endif
#ifdef method_straight

    dest = blend_straight(src, dest, amount);

#endif
#ifdef method_onto

	dest.rgb = (src.rgb - dest.rgb)*(src.a*amount) + dest.rgb;

#endif
#ifdef method_straightonto

    src.a = src.a * dest.a;
    dest = blend_straight(src, dest, amount);

#endif
#ifdef method_brighten

	dest.rgb = max(dest.rgb, src.rgb*(src.a*amount));

#endif
#ifdef method_darken

	float alpha = src.a * amount;
	dest.r = min(dest.r, (src.r - 1) * alpha + 1);
	dest.g = min(dest.g, (src.g - 1) * alpha + 1);
	dest.b = min(dest.b, (src.b - 1) * alpha + 1);

#endif
#ifdef method_add

	dest.rgb = (dest.rgb*dest.a + src.rgb*(src.a*amount));

#endif
#ifdef method_subtract

	dest.rgb = (dest.rgb * dest.a - src.rgb * src.a * amount);

#endif
#ifdef method_difference

	dest.rgb = abs(dest.rgb*dest.a - src.rgb*(src.a*amount));

#endif
#ifdef method_multiply

	if (amount < 0) src = vec4(1) - src;
	dest.rgb = (dest.rgb*src.rgb - dest.rgb)*(src.a*abs(amount)) + dest.rgb;

#endif
#ifdef method_divide

	src.rgb = sign(sign(src.rgb) + vec3(epsilon))*max(abs(src.rgb), vec3(epsilon));
	dest.rgb = (dest.rgb/src.rgb - dest.rgb)*(src.a*amount) + dest.rgb;
	
#endif
#ifdef method_color

	vec4 tmp = vec4( vec3( dot(dest.rgb, encode_yuv[0]), 
					       dot(src.rgb, encode_yuv[1]),
					       dot(src.rgb, encode_yuv[2]) ) * decode_yuv,
					 dest.a );
	dest = (tmp - dest)*(amount*src.a) + dest;

#endif
#ifdef method_luminance

	vec4 tmp = vec4( vec3( dot(src.rgb, encode_yuv[0]), 
					       dot(dest.rgb, encode_yuv[1]),
					       dot(dest.rgb, encode_yuv[2]) ) * decode_yuv,
					 dest.a );
	dest = (tmp - dest)*(amount*src.a) + dest;
	
#endif
#ifdef method_hue

	vec4 temp = dest;
	float h = atan(get_u(src), get_v(src));
	float s = get_s(dest);
	float u = s * sin(h);
	float v = s * cos(h);

	temp.rgb = from_yuv(get_y(temp), u, v);

	dest = (temp - dest) * amount * src.a + dest;
	
#endif
#ifdef method_saturation

    vec4 temp = set_s(dest, get_s(src));
    dest = (temp - dest) * amount * src.a + dest;

#endif
#ifdef method_alphabrighten

	if (src.a < dest.a*amount)
		dest = vec4(src.rgb, src.a*amount);

#endif
#ifdef method_alphadarken

	if (src.a*amount > dest.a)
		dest = vec4(src.rgb, src.a*amount);

#endif
#ifdef method_screen

	if (amount < 0) src = vec4(1) - src;
	src.rgb = vec3(1) - (vec3(1) - src.rgb)*(vec3(1) - dest.rgb);
	dest.rgb = (src.rgb - dest.rgb)*(src.a*abs(amount)) + dest.rgb;

#endif
#ifdef method_overlay

    float amt = amount;
	if (amt < 0) {
        src.rgb = vec3(1) - src.rgb;
        amt = -amt;
    }

    vec3 rm = dest.rgb * src.rgb;
    vec3 rs = vec3(1) - ((vec3(1) - src.rgb) * (vec3(1) - dest.rgb));
    vec4 ret = src;
    ret.rgb = src.rgb * rs + ((vec3(1) - src.rgb) * rm.rgb);

	dest.rgb = (ret.rgb - dest.rgb)*(ret.a*amt) + dest.rgb;

#endif
#ifdef method_hardlight

	if (amount < 0) src = vec4(1) - src;
	vec3 g = vec3(greaterThan(src.rgb, vec3(0.5)));
	src.rgb = g*(vec3(1) - vec3(2)*(vec3(1) - src.rgb)*(vec3(1) - dest.rgb))
	        + (vec3(1) - g)*vec3(2)*src.rgb*dest.rgb;
	dest.rgb = (src.rgb - dest.rgb)*(src.a*abs(amount)) + dest.rgb;

#endif
#ifdef method_alphaover

	vec4 rm = dest;
	rm.a = (1 - src.a) * dest.a;
    dest = blend_straight(rm, dest, amount);

#endif
#ifdef method_add_composite

	float alpha = max(0.f, min(1.f, dest.a + src.a));
	float k = abs(alpha) > 1e-8 ? 1.0/alpha : 0.0;
	dest.a = alpha;
	dest.rgb = k*(dest.rgb * dest.a + src.rgb * src.a);

#endif
#ifdef method_alpha

	vec4 rm = dest;
	rm.a = src.a * dest.a;
    dest = blend_straight(rm, dest, amount);

#endif

	out_color = dest;
}
